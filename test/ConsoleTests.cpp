#include <vix/console.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace console_tests
{
  enum class SampleEnum
  {
    Value = 7
  };

  struct FormatterOnly
  {
    int value;
  };

  struct AdlOnly
  {
    int value;
  };

  struct InspectedOnly
  {
    int value;
  };

  struct ReflectedOnly
  {
    std::string name;
    int count;
  };

  inline void vix_format(std::ostream &os, const AdlOnly &value)
  {
    os << "adl-only(" << value.value << ')';
  }
} // namespace console_tests

template <>
struct vix::formatter<console_tests::FormatterOnly>
{
  static void format(std::ostream &os,
                     const console_tests::FormatterOnly &value)
  {
    os << "formatter-only(" << value.value << ')';
  }
};

template <>
struct vix::inspector<console_tests::InspectedOnly>
{
  static void inspect(vix::inspect_context &ctx,
                      const console_tests::InspectedOnly &value)
  {
    ctx.os << "inspected-only(" << value.value << ')';
  }
};

template <>
struct vix::field_map<console_tests::ReflectedOnly>
{
  static constexpr auto fields()
  {
    return vix::fields(
        vix::field("name", &console_tests::ReflectedOnly::name),
        vix::field("count", &console_tests::ReflectedOnly::count));
  }
};

namespace
{
  using Level = vix::Console::Level;
  using Format = vix::log::LogFormat;

  void fail(const std::string &message)
  {
    std::cerr << "[FAIL] " << message << '\n';
    std::exit(EXIT_FAILURE);
  }

  void require(bool condition, const std::string &message)
  {
    if (!condition)
      fail(message);
  }

  [[nodiscard]] bool contains(std::string_view text, std::string_view needle)
  {
    return text.find(needle) != std::string_view::npos;
  }

  [[nodiscard]] std::string strip_ansi(std::string_view input)
  {
    std::string out;
    out.reserve(input.size());

    for (std::size_t index = 0; index < input.size(); ++index)
    {
      if (input[index] == '\033' && index + 1 < input.size() &&
          input[index + 1] == '[')
      {
        index += 2;
        while (index < input.size() &&
               !((input[index] >= 'A' && input[index] <= 'Z') ||
                 (input[index] >= 'a' && input[index] <= 'z')))
        {
          ++index;
        }
        continue;
      }

      out.push_back(input[index]);
    }

    return out;
  }

  [[nodiscard]] std::vector<std::string> non_empty_lines(std::string_view input)
  {
    std::vector<std::string> lines;
    std::istringstream stream{std::string(input)};
    std::string line;

    while (std::getline(stream, line))
    {
      if (!line.empty())
        lines.push_back(line);
    }

    return lines;
  }

  void flush_logger()
  {
    if (auto logger = spdlog::default_logger())
      logger->flush();
    std::fflush(stdout);
  }

  class StdoutCapture
  {
  public:
    StdoutCapture()
    {
#ifndef _WIN32
      std::fflush(stdout);
      saved_fd_ = ::dup(STDOUT_FILENO);
      require(saved_fd_ >= 0, "dup(stdout) should succeed.");
      require(::pipe(pipe_fd_) == 0, "pipe() should succeed.");
      require(::dup2(pipe_fd_[1], STDOUT_FILENO) >= 0,
              "dup2(pipe write, stdout) should succeed.");
      ::close(pipe_fd_[1]);
      pipe_fd_[1] = -1;
#endif
    }

    StdoutCapture(const StdoutCapture &) = delete;
    StdoutCapture &operator=(const StdoutCapture &) = delete;

    ~StdoutCapture()
    {
#ifndef _WIN32
      if (!restored_)
        (void)read();
#endif
    }

    [[nodiscard]] std::string read()
    {
#ifdef _WIN32
      return {};
#else
      if (restored_)
        return output_;

      flush_logger();
      require(::dup2(saved_fd_, STDOUT_FILENO) >= 0,
              "restoring stdout should succeed.");
      ::close(saved_fd_);
      saved_fd_ = -1;
      restored_ = true;

      char buffer[4096];
      for (;;)
      {
        const ssize_t count = ::read(pipe_fd_[0], buffer, sizeof(buffer));
        if (count > 0)
        {
          output_.append(buffer, static_cast<std::size_t>(count));
          continue;
        }
        break;
      }

      ::close(pipe_fd_[0]);
      pipe_fd_[0] = -1;
      return output_;
#endif
    }

  private:
    int saved_fd_{-1};
    int pipe_fd_[2]{-1, -1};
    bool restored_{false};
    std::string output_;
  };

  struct ConsoleGuard
  {
    vix::Console::Config config{vix::console.config()};
    vix::log::LogContext context{vix::console.context()};

    ConsoleGuard()
    {
      vix::console.set_async(false);
      vix::console.set_format(Format::JSON);
      vix::console.set_level(Level::Trace);
      vix::console.set_rate_limit({false, 0});
      vix::console.clear_context();
      vix::console.set_limits({});
    }

    ~ConsoleGuard()
    {
      vix::console.set_async(false);
      vix::console.clear_context();
      vix::console.set_context(context);
      vix::console.configure(config);
      flush_logger();
    }
  };

  class EnvGuard
  {
  public:
    explicit EnvGuard(std::vector<std::string> names) : names_(std::move(names))
    {
      for (const auto &name : names_)
      {
        const char *value = std::getenv(name.c_str());
        previous_[name] = value == nullptr ? std::optional<std::string>{}
                                           : std::optional<std::string>{value};
      }
    }

    ~EnvGuard()
    {
      for (const auto &name : names_)
      {
        const auto it = previous_.find(name);
        if (it == previous_.end() || !it->second.has_value())
          unset(name);
        else
          set(name, *it->second);
      }
    }

    static void set(const std::string &name, const std::string &value)
    {
#ifdef _WIN32
      _putenv_s(name.c_str(), value.c_str());
#else
      require(::setenv(name.c_str(), value.c_str(), 1) == 0,
              "setenv(" + name + ") should succeed.");
#endif
    }

    static void unset(const std::string &name)
    {
#ifdef _WIN32
      _putenv_s(name.c_str(), "");
#else
      require(::unsetenv(name.c_str()) == 0,
              "unsetenv(" + name + ") should succeed.");
#endif
    }

  private:
    std::vector<std::string> names_;
    std::unordered_map<std::string, std::optional<std::string>> previous_;
  };

  struct JsonValue
  {
    enum class Kind
    {
      String,
      Number,
      Bool
    };

    Kind kind;
    std::string text;
  };

  [[nodiscard]] std::map<std::string, JsonValue> parse_flat_json(
      std::string_view line)
  {
    std::map<std::string, JsonValue> out;
    std::size_t index = 0;

    auto skip_ws = [&]()
    {
      while (index < line.size() &&
             (line[index] == ' ' || line[index] == '\n' ||
              line[index] == '\r' || line[index] == '\t'))
      {
        ++index;
      }
    };

    auto parse_string = [&]() -> std::string
    {
      require(index < line.size() && line[index] == '"',
              "JSON string should start with a quote.");
      ++index;
      std::string result;
      while (index < line.size())
      {
        const char ch = line[index++];
        if (ch == '"')
          return result;
        if (ch == '\\')
        {
          require(index < line.size(), "JSON escape should have a payload.");
          const char escaped = line[index++];
          switch (escaped)
          {
          case '"':
          case '\\':
          case '/':
            result.push_back(escaped);
            break;
          case 'n':
            result.push_back('\n');
            break;
          case 'r':
            result.push_back('\r');
            break;
          case 't':
            result.push_back('\t');
            break;
          default:
            result.push_back(escaped);
            break;
          }
          continue;
        }
        result.push_back(ch);
      }
      fail("JSON string should terminate.");
      return {};
    };

    skip_ws();
    require(index < line.size() && line[index] == '{',
            "JSON object should start with '{'.");
    ++index;

    for (;;)
    {
      skip_ws();
      require(index < line.size(), "JSON object should not end abruptly.");
      if (line[index] == '}')
        break;

      const std::string key = parse_string();
      skip_ws();
      require(index < line.size() && line[index] == ':',
              "JSON key should be followed by ':'.");
      ++index;
      skip_ws();

      JsonValue value{JsonValue::Kind::String, {}};
      if (line[index] == '"')
      {
        value.kind = JsonValue::Kind::String;
        value.text = parse_string();
      }
      else if (line.substr(index, 4) == "true")
      {
        value.kind = JsonValue::Kind::Bool;
        value.text = "true";
        index += 4;
      }
      else if (line.substr(index, 5) == "false")
      {
        value.kind = JsonValue::Kind::Bool;
        value.text = "false";
        index += 5;
      }
      else
      {
        value.kind = JsonValue::Kind::Number;
        const std::size_t start = index;
        while (index < line.size() &&
               (std::isdigit(static_cast<unsigned char>(line[index])) ||
                line[index] == '-' || line[index] == '+' ||
                line[index] == '.' || line[index] == 'e' ||
                line[index] == 'E'))
        {
          ++index;
        }
        value.text = std::string(line.substr(start, index - start));
      }

      out[key] = std::move(value);
      skip_ws();
      if (index < line.size() && line[index] == ',')
      {
        ++index;
        continue;
      }
      require(index < line.size() && line[index] == '}',
              "JSON field should end with ',' or '}'.");
    }

    return out;
  }

  [[nodiscard]] std::map<std::string, JsonValue> one_json_record(
      std::string_view output)
  {
    const auto lines = non_empty_lines(output);
    require(lines.size() == 1, "Expected exactly one JSON log record.");
    return parse_flat_json(lines.front());
  }

  [[nodiscard]] std::string capture_message(auto &&fn)
  {
    StdoutCapture capture;
    fn();
    return one_json_record(capture.read()).at("msg").text;
  }

  void test_public_surface()
  {
    static_assert(noexcept(std::declval<vix::Console &>().log("x")));
    static_assert(noexcept(std::declval<vix::Console &>().info("x")));
    static_assert(noexcept(std::declval<vix::Console &>().debug("x")));
    static_assert(noexcept(std::declval<vix::Console &>().trace("x")));
    static_assert(noexcept(std::declval<vix::Console &>().warn("x")));
    static_assert(noexcept(std::declval<vix::Console &>().error("x")));
    static_assert(noexcept(std::declval<vix::Console &>().critical("x")));
    static_assert(noexcept(std::declval<vix::Console &>().logf("x {}", 1)));
    static_assert(noexcept(std::declval<vix::Console &>().debugf("x {}", 1)));
    static_assert(noexcept(std::declval<vix::Console &>().warnf("x {}", 1)));
    static_assert(noexcept(std::declval<vix::Console &>().errorf("x {}", 1)));
    static_assert(noexcept(std::declval<vix::Console &>().dir(1)));
    static_assert(noexcept(std::declval<vix::Console &>().event("x")));
    static_assert(noexcept(std::declval<vix::Console &>().event_at(Level::Info, "x")));
    static_assert(noexcept(std::declval<vix::Console &>().set_level(Level::Info)));
    static_assert(noexcept(std::declval<vix::Console &>().set_format(Format::JSON)));
    static_assert(noexcept(std::declval<vix::Console &>().set_async(false)));
    static_assert(noexcept(std::declval<vix::Console &>().set_limits({})));
    static_assert(noexcept(std::declval<vix::Console &>().set_rate_limit({false, 0})));
    static_assert(noexcept(std::declval<vix::Console &>().set_context({})));
    static_assert(noexcept(std::declval<vix::Console &>().clear_context()));
    static_assert(!std::is_copy_constructible_v<vix::Console>);
    static_assert(!std::is_copy_assignable_v<vix::Console>);
    static_assert(!std::is_move_constructible_v<vix::Console>);
    static_assert(!std::is_move_assignable_v<vix::Console>);
  }

  void test_parse_level()
  {
    require(vix::Console::parse_level("trace") == Level::Trace, "trace");
    require(vix::Console::parse_level("TRACE") == Level::Trace, "TRACE");
    require(vix::Console::parse_level("debug") == Level::Debug, "debug");
    require(vix::Console::parse_level("info") == Level::Info, "info");
    require(vix::Console::parse_level("log") == Level::Info, "log");
    require(vix::Console::parse_level("warn") == Level::Warn, "warn");
    require(vix::Console::parse_level("warning") == Level::Warn, "warning");
    require(vix::Console::parse_level("error") == Level::Error, "error");
    require(vix::Console::parse_level("err") == Level::Error, "err");
    require(vix::Console::parse_level("critical") == Level::Critical, "critical");
    require(vix::Console::parse_level("fatal") == Level::Critical, "fatal");
    require(vix::Console::parse_level("off") == Level::Off, "off");
    require(vix::Console::parse_level("none") == Level::Off, "none");
    require(vix::Console::parse_level("silent") == Level::Off, "silent");
    require(vix::Console::parse_level("never") == Level::Off, "never");
    require(vix::Console::parse_level("0") == Level::Off, "0");
    require(vix::Console::parse_level("unknown") == Level::Info, "unknown");
  }

  void test_level_filtering()
  {
    ConsoleGuard guard;
    vix::console.set_level(Level::Warn);

    require(!vix::console.enabled(Level::Trace), "trace disabled at warn.");
    require(!vix::console.enabled(Level::Debug), "debug disabled at warn.");
    require(!vix::console.enabled(Level::Info), "info disabled at warn.");
    require(vix::console.enabled(Level::Warn), "warn enabled at warn.");
    require(vix::console.enabled(Level::Error), "error enabled at warn.");
    require(vix::console.enabled(Level::Critical), "critical enabled at warn.");
    require(!vix::console.enabled(Level::Off), "off is never enabled.");

    StdoutCapture capture;
    vix::console.trace("trace hidden");
    vix::console.debug("debug hidden");
    vix::console.log("log hidden");
    vix::console.info("info hidden");
    vix::console.warn("warn shown");
    vix::console.error("error shown");
    vix::console.critical("critical shown");
    const auto lines = non_empty_lines(capture.read());
    require(lines.size() == 3, "Warn, error and critical should log.");

    vix::console.set_level(Level::Off);
    StdoutCapture off_capture;
    vix::console.critical("hidden");
    require(off_capture.read().empty(), "Level::Off should suppress all output.");
  }

  void test_configuration()
  {
    ConsoleGuard guard;
    vix::Console::Limits tiny{0, 0, 0, 0};
    vix::console.set_limits(tiny);
    const auto limits = vix::console.limits();
    require(limits.max_depth == 1, "max_depth should clamp to 1.");
    require(limits.max_items == 1, "max_items should clamp to 1.");
    require(limits.max_string_length == 16,
            "max_string_length should clamp to 16.");
    require(limits.max_record_size == 128,
            "max_record_size should clamp to 128.");

    vix::console.set_level(Level::Debug);
    require(vix::console.level() == Level::Debug, "level should round-trip.");
    vix::console.set_format(Format::JSON_PRETTY);
    require(vix::console.format() == Format::JSON_PRETTY,
            "format should round-trip.");
    vix::console.set_async(true);
    require(vix::console.async(), "async should be enabled.");
    vix::console.set_async(false);
    require(!vix::console.async(), "async should be restored.");

    vix::console.set_rate_limit({true, 5});
    auto rate = vix::console.rate_limit();
    require(rate.enabled && rate.max_per_second == 5,
            "rate limit should round-trip.");

    vix::Console::Config config;
    config.level = Level::Error;
    config.format = Format::JSON;
    config.async = false;
    config.rate_limit = {false, 0};
    config.limits = {3, 4, 64, 512};
    vix::console.configure(config);
    const auto copy = vix::console.config();
    require(copy.level == Level::Error && copy.format == Format::JSON &&
                !copy.async && !copy.rate_limit.enabled &&
                copy.limits.max_depth == 3 && copy.limits.max_items == 4,
            "configure/config should round-trip.");
  }

  void test_primitives()
  {
    ConsoleGuard guard;
    const char *null_cstr = nullptr;
    const std::string msg = capture_message(
        [&]()
        { vix::console.log("user", 42, true, 3.14, nullptr); });
    require(contains(msg, "user 42 true") && contains(msg, "3.14") &&
                contains(msg, " null"),
            "primitive message should be space-separated.");
    require(!contains(msg, "\"user\""), "human strings should not be quoted.");

    const std::string more = capture_message(
        [&]()
        {
          vix::console.log(false, -7, 42u, console_tests::SampleEnum::Value,
                           null_cstr);
        });
    require(contains(more, "false -7 42 7 null"),
            "bools, ints, enums and null C strings should render.");

    StdoutCapture capture;
    vix::console.log();
    require(non_empty_lines(capture.read()).size() == 1,
            "log() without arguments should produce one empty message record.");
  }

  void test_collections_and_complex_types()
  {
    ConsoleGuard guard;
    const std::vector<int> vec{1, 2, 3};
    const std::map<std::string, int> map{{"a", 1}, {"b", 2}};
    const std::unordered_map<std::string, int> unordered{{"x", 1}, {"y", 2}};
    const auto tuple = std::make_tuple(1, std::string{"two"}, true);
    const std::pair<std::string, int> pair{"age", 42};
    const std::optional<int> optional{9};
    const std::variant<int, std::string> variant{std::string{"variant"}};
    const auto unique = std::make_unique<int>(5);
    const auto shared = std::make_shared<std::string>("shared");
    const std::chrono::milliseconds duration{25};
    const std::filesystem::path path{"alpha/beta.txt"};
    const std::map<std::string, std::vector<int>> nested{{"ids", {1, 2, 3}}};

    const std::string msg = capture_message(
        [&]()
        {
          vix::console.log(vec, map, unordered, tuple, pair, optional, variant,
                           unique, shared, duration, path, nested);
        });

    require(!contains(msg, "[object]"), "complex values must not be [object].");
    for (std::string_view needle :
         {"1", "a", "x", "two", "age", "9", "variant", "5", "shared",
          "25", "alpha", "ids"})
    {
      require(contains(msg, needle), "complex output should contain expected value.");
    }
  }

  void test_custom_extensions()
  {
    ConsoleGuard guard;
    const std::string formatted = capture_message(
        [&]()
        { vix::console.log(console_tests::FormatterOnly{3}); });
    require(contains(formatted, "formatter-only(3)"),
            "formatter<T> should be used.");

    const std::string adl = capture_message(
        [&]()
        { vix::console.log(console_tests::AdlOnly{4}); });
    require(contains(adl, "adl-only(4)"), "ADL vix_format should be used.");

    const std::string inspected = capture_message(
        [&]()
        { vix::console.dir(console_tests::InspectedOnly{5}); });
    require(contains(inspected, "inspected-only(5)"),
            "inspector<T> should be used by dir().");

    const std::string reflected = capture_message(
        [&]()
        { vix::console.dir(console_tests::ReflectedOnly{"items", 6}); });
    require(contains(reflected, "name") && contains(reflected, "items") &&
                contains(reflected, "count") && contains(reflected, "6"),
            "field_map<T> should be used by dir().");
  }

  void test_formatting()
  {
    ConsoleGuard guard;
    require(contains(capture_message([&]()
                                     { vix::console.logf("user {} connected", 42); }),
                     "user 42 connected"),
            "logf should format.");
    require(contains(capture_message([&]()
                                     { vix::console.debugf("value={}", 10); }),
                     "value=10"),
            "debugf should format.");
    const std::string warn_message = capture_message(
        [&]()
        { vix::console.warnf("warning {}", "message"); });
    require(contains(warn_message, "warning") && contains(warn_message, "message"),
            "warnf should format: " + warn_message);
    require(contains(capture_message([&]()
                                     { vix::console.errorf("error {}", 500); }),
                     "error 500"),
            "errorf should format.");

    const std::string invalid = capture_message(
        [&]()
        { vix::console.logf("invalid {"); });
    require(contains(invalid, "<console-render-error"),
            "invalid format should be caught and logged.");
  }

  void test_dir()
  {
    ConsoleGuard guard;
    std::map<std::string, std::vector<int>> value{{"numbers", {1, 2, 3}}};

    const std::string default_dir = capture_message(
        [&]()
        { vix::console.dir(value); });
    require(contains(default_dir, "\n"),
            "dir() should render multiple lines: " + default_dir);
    require(contains(default_dir, "numbers") && contains(default_dir, "[n="),
            "dir() should show nested collections and metadata.");

    vix::inspect_options options = vix::default_options();
    options.max_depth = 100;
    options.max_items = 100;
    options.compact = false;
    vix::console.set_limits({2, 1, 128, 1024});
    const std::string custom = capture_message(
        [&]()
        { vix::console.dir(value, options); });
    require(contains(custom, "<depth-limit>") || contains(custom, "..."),
            "dir() custom options should be clamped by console limits.");

    vix::console.set_limits({});
    const std::string reflected = capture_message(
        [&]()
        { vix::console.dir(console_tests::ReflectedOnly{"dir", 8}); });
    require(contains(reflected, "dir") && contains(reflected, "count"),
            "dir() should handle field_map types.");
  }

  void test_limits_and_truncation()
  {
    ConsoleGuard guard;
    vix::console.set_limits({2, 2, 16, 128});
    const std::string long_string(40, 'a');
    const std::string truncated = capture_message(
        [&]()
        { vix::console.log(long_string); });
    require(contains(truncated, "...<truncated>"),
            "long strings should be truncated.");

    vix::console.set_limits({4, 10, 256, 128});
    const std::string bounded = capture_message(
        [&]()
        { vix::console.log(std::string(400, 'b')); });
    require(bounded.size() <= 128, "msg should be bounded by max_record_size.");

    vix::console.set_limits({4, 2, 256, 1024});
    const std::string items = capture_message(
        [&]()
        { vix::console.log(std::vector<int>{1, 2, 3, 4, 5}); });
    require(!contains(items, "5"), "max_items should limit collection output.");

    vix::console.set_limits({1, 10, 256, 1024});
    const std::map<std::string, std::vector<int>> deep{{"k", {1, 2, 3}}};
    const std::string depth = capture_message([&]()
                                              { vix::console.log(deep); });
    require(contains(depth, "<depth-limit>"),
            "max_depth should produce inspect depth-limit marker.");
  }

  void test_context()
  {
    ConsoleGuard guard;
    vix::log::LogContext context;
    context.request_id = "req-123";
    context.module = "http";
    context.fields["tenant"] = "softadastra";
    vix::console.set_context(context);

    const auto current = vix::console.context();
    require(current.request_id == "req-123" && current.module == "http" &&
                current.fields.at("tenant") == "softadastra",
            "context() should return a copy of the active context.");

    StdoutCapture capture;
    vix::console.log("request completed");
    const auto record = one_json_record(capture.read());
    require(record.at("rid").text == "req-123", "rid should be present.");
    require(record.at("mod").text == "http", "mod should be present.");
    require(record.at("tenant").text == "softadastra",
            "custom context field should be present.");

    vix::console.clear_context();
    StdoutCapture clear_capture;
    vix::console.log("request completed");
    const auto cleared = one_json_record(clear_capture.read());
    require(cleared.count("rid") == 0 && cleared.count("mod") == 0 &&
                cleared.count("tenant") == 0,
            "clear_context() should remove context fields.");
  }

  void test_structured_events()
  {
    ConsoleGuard guard;
    int status = 200;
    std::vector<int> values{1, 2, 3};
    StdoutCapture capture;
    vix::console.event(
        "http.request",
        vix::Console::field("method", "GET"),
        vix::Console::field("status", status),
        vix::Console::field("success", true),
        vix::Console::field("duration_ms", 4.5),
        vix::Console::field("values", values),
        vix::Console::field("", 42));
    const auto record = one_json_record(capture.read());
    require(record.at("msg").text == "http.request", "event name should be msg.");
    require(record.at("method").text == "GET",
            "method should be present: " + record.at("method").text);
    require(record.at("status").kind == JsonValue::Kind::Number &&
                record.at("status").text == "200",
            "status should stay numeric.");
    require(record.at("success").kind == JsonValue::Kind::Bool &&
                record.at("success").text == "true",
            "success should stay bool.");
    require(record.at("duration_ms").kind == JsonValue::Kind::Number,
            "duration_ms should stay numeric.");
    require(record.at("values").kind == JsonValue::Kind::String &&
                contains(record.at("values").text, "1"),
            "complex event values should become bounded strings.");
    require(record.at("field").text == "42", "empty event key should normalize.");

    StdoutCapture levels;
    vix::console.event_at(Level::Warn, "warn.event");
    vix::console.event_at(Level::Error, "error.event");
    vix::console.event_at(Level::Off, "off.event");
    const auto lines = non_empty_lines(levels.read());
    require(lines.size() == 2, "Warn and Error events should log; Off should not.");
  }

  void test_field_lifetime()
  {
    ConsoleGuard guard;
    int status = 200;
    auto ref = vix::Console::field("status", status);
    static_assert(std::is_same_v<decltype(ref), vix::Console::FieldRef<int>>);
    status = 201;

    StdoutCapture ref_capture;
    vix::console.event("response", ref);
    require(one_json_record(ref_capture.read()).at("status").text == "201",
            "FieldRef should observe lvalue changes.");

    auto owned = vix::Console::field("name", std::string{"temporary"});
    static_assert(
        std::is_same_v<decltype(owned), vix::Console::FieldValue<std::string>>);
    StdoutCapture owned_capture;
    vix::console.event("value", owned);
    require(one_json_record(owned_capture.read()).at("name").text == "temporary",
            "FieldValue should own rvalue data.");
  }

  void test_rate_limiting()
  {
    ConsoleGuard guard;
    vix::console.set_rate_limit({false, 1});
    StdoutCapture disabled;
    vix::console.log("a");
    vix::console.log("b");
    vix::console.log("c");
    require(non_empty_lines(disabled.read()).size() == 3,
            "disabled rate limiting should let all messages through.");

    vix::console.set_rate_limit({true, 0});
    StdoutCapture zero;
    vix::console.log("a");
    vix::console.log("b");
    require(non_empty_lines(zero.read()).size() == 2,
            "max_per_second=0 should disable rate limiting.");

    vix::console.set_rate_limit({true, 1});
    StdoutCapture limited;
    vix::console.trace("trace one");
    vix::console.debug("debug two");
    vix::console.info("info three");
    vix::console.warn("warn kept");
    vix::console.error("error kept");
    vix::console.critical("critical kept");
    const std::string output = limited.read();
    const auto lines = non_empty_lines(output);
    require(lines.size() >= 4 && lines.size() <= 5,
            "low severities should be limited while warn+ are preserved.");
    const bool all_low_present = contains(output, "trace one") &&
                                 contains(output, "debug two") &&
                                 contains(output, "info three");
    require(!all_low_present, "rate limiting should suppress low severities.");
    require(contains(output, "warn kept") && contains(output, "error kept") &&
                contains(output, "critical kept"),
            "Warn, Error and Critical should bypass rate limiting.");
  }

  void test_concurrency()
  {
    ConsoleGuard guard;
    constexpr int thread_count = 4;
    constexpr int messages_per_thread = 4;
    StdoutCapture capture;
    std::vector<std::thread> threads;

    for (int thread_index = 0; thread_index < thread_count; ++thread_index)
    {
      threads.emplace_back(
          [thread_index]()
          {
            vix::log::LogContext context;
            context.request_id = "req-" + std::to_string(thread_index);
            context.module = "worker";
            context.fields["thread"] = std::to_string(thread_index);
            vix::console.set_context(context);

            for (int message = 0; message < messages_per_thread; ++message)
            {
              vix::console.log("thread", thread_index, "message", message);
            }

            vix::console.clear_context();
          });
    }

    for (auto &thread : threads)
      thread.join();

    const auto lines = non_empty_lines(capture.read());
    require(lines.size() == thread_count * messages_per_thread,
            "concurrent logging should produce complete lines.");

    std::vector<int> seen(thread_count, 0);
    for (const auto &line : lines)
    {
      const auto record = parse_flat_json(line);
      const auto thread_it = record.find("thread");
      require(thread_it != record.end(), "thread context should be present.");
      const int thread_index = std::stoi(thread_it->second.text);
      require(record.at("rid").text == "req-" + std::to_string(thread_index),
              "thread-local request ids should not mix.");
      require(contains(record.at("msg").text,
                       "thread " + std::to_string(thread_index)),
              "message should contain the expected thread id.");
      ++seen.at(static_cast<std::size_t>(thread_index));
    }

    for (int count : seen)
      require(count == messages_per_thread, "each thread id should be present.");
  }

  void test_environment()
  {
    const std::vector<std::string> names{
        "VIX_CONSOLE_LEVEL",
        "VIX_CONSOLE_RATE_LIMIT",
        "VIX_CONSOLE_MAX_DEPTH",
        "VIX_CONSOLE_MAX_ITEMS",
        "VIX_CONSOLE_MAX_STRING_LENGTH",
        "VIX_CONSOLE_MAX_RECORD_SIZE",
        "VIX_LOG_FORMAT"};
    EnvGuard env(names);
    for (const auto &name : names)
      EnvGuard::unset(name);

    EnvGuard::set("VIX_CONSOLE_LEVEL", "debug");
    EnvGuard::set("VIX_CONSOLE_RATE_LIMIT", "12");
    EnvGuard::set("VIX_CONSOLE_MAX_DEPTH", "3");
    EnvGuard::set("VIX_CONSOLE_MAX_ITEMS", "4");
    EnvGuard::set("VIX_CONSOLE_MAX_STRING_LENGTH", "64");
    EnvGuard::set("VIX_CONSOLE_MAX_RECORD_SIZE", "512");
    EnvGuard::set("VIX_LOG_FORMAT", "json");
    vix::Console configured;
    require(configured.level() == Level::Debug, "env level should be applied.");
    require(configured.format() == Format::JSON, "env format should be applied.");
    require(configured.rate_limit().enabled &&
                configured.rate_limit().max_per_second == 12,
            "env rate limit should be applied.");
    require(configured.limits().max_depth == 3 &&
                configured.limits().max_items == 4 &&
                configured.limits().max_string_length == 64 &&
                configured.limits().max_record_size == 512,
            "env limits should be applied.");

    EnvGuard::set("VIX_CONSOLE_LEVEL", "bad");
    EnvGuard::set("VIX_CONSOLE_RATE_LIMIT", "bad");
    EnvGuard::set("VIX_CONSOLE_MAX_DEPTH", "bad");
    EnvGuard::set("VIX_CONSOLE_MAX_ITEMS", "0");
    EnvGuard::set("VIX_CONSOLE_MAX_STRING_LENGTH", "999999999999999999999999");
    EnvGuard::set("VIX_CONSOLE_MAX_RECORD_SIZE", "bad");
    vix::Console fallback;
    require(fallback.level() == Level::Info,
            "invalid console level should fall back to Info.");
    require(!fallback.rate_limit().enabled,
            "invalid/zero rate limit should be disabled.");
    require(fallback.limits().max_depth >= 1 &&
                fallback.limits().max_items >= 1 &&
                fallback.limits().max_string_length >= 16 &&
                fallback.limits().max_record_size >= 128,
            "invalid limits should fall back to clamped defaults.");

    EnvGuard::set("VIX_CONSOLE_LEVEL", "0");
    vix::Console off;
    require(off.level() == Level::Off, "zero level should map to Off.");
  }

  void test_formats()
  {
    ConsoleGuard guard;
    vix::console.set_format(Format::KV);
    StdoutCapture kv_capture;
    vix::console.warn("kv message");
    const std::string kv = strip_ansi(kv_capture.read());
    require(contains(kv, "[vix]") &&
                (contains(kv, "[warn]") || contains(kv, "[warning]")) &&
                contains(kv, "kv message"),
            "KV format should include level and message: " + kv);

    vix::console.set_format(Format::JSON);
    StdoutCapture json_capture;
    vix::console.event("json.event", vix::Console::field("status", 200),
                       vix::Console::field("success", true));
    const auto json_lines = non_empty_lines(json_capture.read());
    require(json_lines.size() == 1 && !contains(json_lines.front(), "\n"),
            "JSON format should be single-line.");
    const auto json = parse_flat_json(json_lines.front());
    require(json.at("status").kind == JsonValue::Kind::Number &&
                json.at("success").kind == JsonValue::Kind::Bool,
            "JSON structured fields should keep primitive types.");

    vix::console.set_format(Format::JSON_PRETTY);
    StdoutCapture pretty_capture;
    vix::console.event("pretty.event", vix::Console::field("status", 200));
    const std::string pretty = strip_ansi(pretty_capture.read());
    require(contains(pretty, "{\n") && contains(pretty, "\"msg\"") &&
                contains(pretty, "pretty.event"),
            "JSON_PRETTY should be multiline JSON.");
  }
} // namespace

int main()
{
  test_public_surface();
  test_parse_level();
  test_level_filtering();
  test_configuration();
  test_primitives();
  test_collections_and_complex_types();
  test_custom_extensions();
  test_formatting();
  test_dir();
  test_limits_and_truncation();
  test_context();
  test_structured_events();
  test_field_lifetime();
  test_rate_limiting();
  test_concurrency();
  test_environment();
  test_formats();

  std::cout << "[PASS] vix_io_console_test\n";
  return EXIT_SUCCESS;
}
