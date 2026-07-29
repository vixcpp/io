/**
 *
 *  @file Console.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira. All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 *
 *  Production-ready JavaScript-like console facade.
 */
#ifndef VIX_CONSOLE_CLASS_HPP
#define VIX_CONSOLE_CLASS_HPP

#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <limits>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>

#include <vix/format.hpp>
#include <vix/inspect.hpp>
#include <vix/log/Log.hpp>
#include <vix/print.hpp>
#include <vix/utils/Env.hpp>

namespace vix
{

  /**
   * @class Console
   * @brief JavaScript-like console API backed by the Vix logging pipeline.
   *
   * Console provides a familiar API (`log`, `info`, `debug`, `warn`, `error`)
   * while delegating transport, filtering, async mode, context, and structured
   * output to `vix::log`.
   *
   * Primitive values use a lightweight rendering path. Complex values are
   * rendered through the Vix print/inspect engines, so STL collections,
   * tuples, optionals, variants, smart pointers, custom formatters, and custom
   * inspectors can be logged naturally.
   */
  class Console final
  {
  public:
    /**
     * @brief Console severity levels.
     *
     * Values are ordered from least severe to most severe so level gating is a
     * single integer comparison.
     */
    enum class Level : std::uint8_t
    {
      Trace = 0,
      Debug = 1,
      Info = 2,
      Warn = 3,
      Error = 4,
      Critical = 5,
      Off = 6
    };

    /**
     * @brief Hard limits applied while rendering console records.
     */
    struct Limits
    {
      int max_depth{6};
      std::size_t max_items{100};
      std::size_t max_string_length{4096};
      std::size_t max_record_size{64u * 1024u};
    };

    /**
     * @brief Optional rate limiting for low-severity console records.
     *
     * Warn, Error, and Critical records are never rate limited.
     */
    struct RateLimit
    {
      bool enabled{false};
      std::uint32_t max_per_second{0};
    };

    /**
     * @brief Complete console configuration.
     */
    struct Config
    {
      Level level{Level::Info};
      vix::log::LogFormat format{vix::log::LogFormat::KV};
      bool async{false};
      Limits limits{};
      RateLimit rate_limit{};
    };

    /**
     * @brief Non-owning field wrapper for lvalue event fields.
     */
    template <typename T>
    struct FieldRef
    {
      std::string_view key;
      const T *value{nullptr};

      [[nodiscard]] std::string_view key_view() const noexcept { return key; }
      [[nodiscard]] const T &get() const noexcept { return *value; }
    };

    /**
     * @brief Owning field wrapper for temporary event fields.
     */
    template <typename T>
    struct FieldValue
    {
      std::string key;
      T value;

      [[nodiscard]] std::string_view key_view() const noexcept { return key; }
      [[nodiscard]] const T &get() const noexcept { return value; }
    };

    /**
     * @brief Construct a field from an lvalue without copying it.
     */
    template <typename T>
    [[nodiscard]] static FieldRef<T> field(std::string_view key, T &value) noexcept
    {
      return FieldRef<T>{key, std::addressof(value)};
    }

    /**
     * @brief Construct an owning field from a temporary value.
     */
    template <typename T>
      requires(!std::is_lvalue_reference_v<T>)
    [[nodiscard]] static FieldValue<std::decay_t<T>> field(std::string_view key, T &&value)
    {
      return FieldValue<std::decay_t<T>>{
          std::string(key),
          std::forward<T>(value)};
    }

    /**
     * @brief Initialize from the active Vix logger and console environment.
     *
     * Supported console-specific variables:
     * - VIX_CONSOLE_LEVEL
     * - VIX_CONSOLE_RATE_LIMIT
     * - VIX_CONSOLE_MAX_DEPTH
     * - VIX_CONSOLE_MAX_ITEMS
     * - VIX_CONSOLE_MAX_STRING_LENGTH
     * - VIX_CONSOLE_MAX_RECORD_SIZE
     *
     * `VIX_LOG_LEVEL` and `VIX_LOG_FORMAT` remain owned by `vix::log`.
     */
    Console() noexcept
    {
      initialize_from_backend_and_env_();
    }

    Console(const Console &) = delete;
    Console &operator=(const Console &) = delete;
    Console(Console &&) = delete;
    Console &operator=(Console &&) = delete;

    // ---------------------------------------------------------------------
    // JavaScript-like logging API
    // ---------------------------------------------------------------------

    template <typename... Ts>
    void log(Ts &&...values) noexcept
    {
      write_(Level::Info, true, std::forward<Ts>(values)...);
    }

    template <typename... Ts>
    void info(Ts &&...values) noexcept
    {
      write_(Level::Info, true, std::forward<Ts>(values)...);
    }

    template <typename... Ts>
    void debug(Ts &&...values) noexcept
    {
      write_(Level::Debug, true, std::forward<Ts>(values)...);
    }

    template <typename... Ts>
    void trace(Ts &&...values) noexcept
    {
      write_(Level::Trace, true, std::forward<Ts>(values)...);
    }

    template <typename... Ts>
    void warn(Ts &&...values) noexcept
    {
      write_(Level::Warn, false, std::forward<Ts>(values)...);
    }

    template <typename... Ts>
    void error(Ts &&...values) noexcept
    {
      write_(Level::Error, false, std::forward<Ts>(values)...);
    }

    template <typename... Ts>
    void critical(Ts &&...values) noexcept
    {
      write_(Level::Critical, false, std::forward<Ts>(values)...);
    }

    /**
     * @brief Format an INFO message with Vix placeholder syntax.
     *
     * Example: `console.logf("user {} connected", user_id);`
     */
    template <typename... Args>
    void logf(std::string_view format_string, const Args &...args) noexcept
    {
      formatted_(Level::Info, true, format_string, args...);
    }

    template <typename... Args>
    void debugf(std::string_view format_string, const Args &...args) noexcept
    {
      formatted_(Level::Debug, true, format_string, args...);
    }

    template <typename... Args>
    void warnf(std::string_view format_string, const Args &...args) noexcept
    {
      formatted_(Level::Warn, false, format_string, args...);
    }

    template <typename... Args>
    void errorf(std::string_view format_string, const Args &...args) noexcept
    {
      formatted_(Level::Error, false, format_string, args...);
    }

    /**
     * @brief Deeply inspect a value and emit it as an INFO record.
     */
    template <typename T>
    void dir(const T &value) noexcept
    {
      inspect_options opts = default_options();
      opts.compact = false;
      opts.show_type = true;
      opts.show_meta = false;
      opts.show_address = false;
      dir(value, opts);
    }

    /**
     * @brief Deeply inspect a value with custom inspect options.
     */
    template <typename T>
    void dir(const T &value, inspect_options options) noexcept
    {
      if (!enabled_(Level::Info))
        return;

      try
      {
        const LimitsSnapshot limits = limits_snapshot_();
        clamp_inspect_options_(options, limits);

        std::string rendered = vix::inspect_to_string(value, options);
        if (!options.compact && rendered.find('\n') == std::string::npos)
          rendered.push_back('\n');
        truncate_record_(rendered, limits.max_record_size);
        submit_(Level::Info, rendered);
      }
      catch (const std::exception &e)
      {
        submit_render_error_(Level::Info, e.what());
      }
      catch (...)
      {
        submit_render_error_(Level::Info, "unknown inspection failure");
      }
    }

    /**
     * @brief Emit a named structured event at INFO level.
     *
     * Primitive field values keep their primitive type in JSON output. Complex
     * field values are rendered to bounded strings through the Vix renderer.
     *
     * Example:
     * @code{.cpp}
     * console.event(
     *     "http.request",
     *     Console::field("method", method),
     *     Console::field("status", 200),
     *     Console::field("request", request));
     * @endcode
     */
    template <typename... Fields>
    void event(std::string_view name, const Fields &...fields) noexcept
    {
      event_at(Level::Info, name, fields...);
    }

    /**
     * @brief Emit a named structured event at an explicit level.
     */
    template <typename... Fields>
    void event_at(Level level_value,
                  std::string_view name,
                  const Fields &...fields) noexcept
    {
      if (!enabled_(level_value))
        return;

      const bool low_severity = is_low_severity_(level_value);
      if (low_severity)
      {
        const RateDecision decision = rate_decision_();
        if (decision.suppressed_to_report > 0)
          report_suppressed_(decision.suppressed_to_report);
        if (!decision.allow)
          return;
      }

      try
      {
        const LimitsSnapshot limits = limits_snapshot_();
        std::string event_name{name};
        truncate_record_(event_name, limits.max_record_size);

        auto prepared = std::make_tuple(prepare_field_(fields, limits)...);
        submit_prepared_event_(
            level_value,
            event_name,
            prepared,
            std::index_sequence_for<Fields...>{});
      }
      catch (const std::exception &e)
      {
        submit_render_error_(level_value, e.what());
      }
      catch (...)
      {
        submit_render_error_(level_value, "unknown event rendering failure");
      }
    }

    // ---------------------------------------------------------------------
    // Configuration and logger integration
    // ---------------------------------------------------------------------

    void configure(const Config &config) noexcept
    {
      set_limits(config.limits);
      set_rate_limit(config.rate_limit);
      set_format(config.format);
      set_async(config.async);
      set_level(config.level);
    }

    [[nodiscard]] Config config() const noexcept
    {
      Config out;
      out.level = level();
      out.format = format();
      out.async = async();
      out.limits = limits();
      out.rate_limit = rate_limit();
      return out;
    }

    void set_level(Level level_value) noexcept
    {
      level_.store(level_value, std::memory_order_release);
      try
      {
        vix::log::set_level(to_log_level_(level_value));
      }
      catch (...)
      {
        // Logging configuration is best-effort and must not terminate the app.
      }
    }

    [[nodiscard]] Level level() const noexcept
    {
      return level_.load(std::memory_order_acquire);
    }

    [[nodiscard]] bool enabled(Level level_value) const noexcept
    {
      return enabled_(level_value);
    }

    void set_format(vix::log::LogFormat format_value) noexcept
    {
      format_.store(format_value, std::memory_order_release);
      try
      {
        vix::log::set_format(format_value);
      }
      catch (...)
      {
      }
    }

    [[nodiscard]] vix::log::LogFormat format() const noexcept
    {
      return format_.load(std::memory_order_acquire);
    }

    void set_async(bool enable) noexcept
    {
      async_.store(enable, std::memory_order_release);
      try
      {
        vix::log::set_async(enable);
      }
      catch (...)
      {
      }
    }

    [[nodiscard]] bool async() const noexcept
    {
      return async_.load(std::memory_order_acquire);
    }

    void set_limits(Limits limits_value) noexcept
    {
      limits_value.max_depth = std::max(limits_value.max_depth, 1);
      limits_value.max_items = std::max<std::size_t>(limits_value.max_items, 1);
      limits_value.max_string_length =
          std::max<std::size_t>(limits_value.max_string_length, 16);
      limits_value.max_record_size =
          std::max<std::size_t>(limits_value.max_record_size, 128);

      max_depth_.store(limits_value.max_depth, std::memory_order_release);
      max_items_.store(limits_value.max_items, std::memory_order_release);
      max_string_length_.store(
          limits_value.max_string_length,
          std::memory_order_release);
      max_record_size_.store(
          limits_value.max_record_size,
          std::memory_order_release);
    }

    [[nodiscard]] Limits limits() const noexcept
    {
      const LimitsSnapshot snapshot = limits_snapshot_();
      return Limits{
          snapshot.max_depth,
          snapshot.max_items,
          snapshot.max_string_length,
          snapshot.max_record_size};
    }

    void set_rate_limit(RateLimit rate_limit_value) noexcept
    {
      const bool enabled_value =
          rate_limit_value.enabled && rate_limit_value.max_per_second > 0;

      rate_limit_max_.store(
          enabled_value ? rate_limit_value.max_per_second : 0,
          std::memory_order_release);
      rate_limit_enabled_.store(enabled_value, std::memory_order_release);

      if (!enabled_value)
      {
        rate_epoch_sec_.store(0, std::memory_order_relaxed);
        rate_count_.store(0, std::memory_order_relaxed);
        rate_suppressed_.store(0, std::memory_order_relaxed);
      }
    }

    [[nodiscard]] RateLimit rate_limit() const noexcept
    {
      return RateLimit{
          rate_limit_enabled_.load(std::memory_order_acquire),
          rate_limit_max_.load(std::memory_order_acquire)};
    }

    void set_context(vix::log::LogContext context_value) noexcept
    {
      try
      {
        vix::log::set_context(std::move(context_value));
      }
      catch (...)
      {
      }
    }

    void clear_context() noexcept
    {
      try
      {
        vix::log::clear_context();
      }
      catch (...)
      {
      }
    }

    [[nodiscard]] vix::log::LogContext context() const
    {
      return vix::log::context();
    }

    /**
     * @brief Parse a console level name.
     */
    [[nodiscard]] static Level parse_level(std::string_view value) noexcept
    {
      if (iequals_(value, "trace"))
        return Level::Trace;
      if (iequals_(value, "debug"))
        return Level::Debug;
      if (iequals_(value, "info") || iequals_(value, "log"))
        return Level::Info;
      if (iequals_(value, "warn") || iequals_(value, "warning"))
        return Level::Warn;
      if (iequals_(value, "error") || iequals_(value, "err"))
        return Level::Error;
      if (iequals_(value, "critical") || iequals_(value, "fatal"))
        return Level::Critical;
      if (iequals_(value, "off") || iequals_(value, "none") ||
          iequals_(value, "silent") || iequals_(value, "never") ||
          value == "0")
        return Level::Off;
      return Level::Info;
    }

  private:
    struct LimitsSnapshot
    {
      int max_depth;
      std::size_t max_items;
      std::size_t max_string_length;
      std::size_t max_record_size;
    };

    struct RateDecision
    {
      bool allow{true};
      std::uint32_t suppressed_to_report{0};
    };

    template <typename T>
    struct PreparedField
    {
      std::string key;
      T value;
    };

    std::atomic<Level> level_{Level::Info};
    std::atomic<vix::log::LogFormat> format_{vix::log::LogFormat::KV};
    std::atomic<bool> async_{false};

    std::atomic<int> max_depth_{6};
    std::atomic<std::size_t> max_items_{100};
    std::atomic<std::size_t> max_string_length_{4096};
    std::atomic<std::size_t> max_record_size_{64u * 1024u};

    std::atomic<bool> rate_limit_enabled_{false};
    std::atomic<std::uint32_t> rate_limit_max_{0};
    std::atomic<std::uint64_t> rate_epoch_sec_{0};
    std::atomic<std::uint32_t> rate_count_{0};
    std::atomic<std::uint32_t> rate_suppressed_{0};

    static constexpr std::string_view kTruncatedMarker{" ...<truncated>"};

    // ---------------------------------------------------------------------
    // Main write paths
    // ---------------------------------------------------------------------

    template <typename... Ts>
    void write_(Level level_value, bool apply_rate_limit, Ts &&...values) noexcept
    {
      if (!enabled_(level_value))
        return;

      if (apply_rate_limit && is_low_severity_(level_value))
      {
        const RateDecision decision = rate_decision_();
        if (decision.suppressed_to_report > 0)
          report_suppressed_(decision.suppressed_to_report);
        if (!decision.allow)
          return;
      }

      try
      {
        const LimitsSnapshot limits = limits_snapshot_();
        std::string message = render_arguments_(limits, std::forward<Ts>(values)...);
        submit_(level_value, message);
      }
      catch (const std::exception &e)
      {
        submit_render_error_(level_value, e.what());
      }
      catch (...)
      {
        submit_render_error_(level_value, "unknown rendering failure");
      }
    }

    template <typename... Args>
    void formatted_(Level level_value,
                    bool apply_rate_limit,
                    std::string_view format_string,
                    const Args &...args) noexcept
    {
      if (!enabled_(level_value))
        return;

      if (apply_rate_limit && is_low_severity_(level_value))
      {
        const RateDecision decision = rate_decision_();
        if (decision.suppressed_to_report > 0)
          report_suppressed_(decision.suppressed_to_report);
        if (!decision.allow)
          return;
      }

      try
      {
        const LimitsSnapshot limits = limits_snapshot_();
        std::string message = vix::format(format_string, args...);
        truncate_record_(message, limits.max_record_size);
        submit_(level_value, message);
      }
      catch (const std::exception &e)
      {
        submit_render_error_(level_value, e.what());
      }
      catch (...)
      {
        submit_render_error_(level_value, "unknown formatting failure");
      }
    }

    void submit_(Level level_value, const std::string &message) noexcept
    {
      try
      {
        // logf() is used intentionally: unlike the basic message path, it also
        // attaches the active Vix request/module context in KV and JSON modes.
        vix::log::global().logf(to_log_level_(level_value), message);
      }
      catch (...)
      {
        // Console is best-effort and must never terminate the application.
      }
    }

    void submit_render_error_(Level level_value, std::string_view reason) noexcept
    {
      try
      {
        std::string message{"<console-render-error"};
        if (!reason.empty())
        {
          message += ": ";
          message.append(reason.data(), reason.size());
        }
        message += ">";
        truncate_record_(message, limits_snapshot_().max_record_size);
        submit_(level_value, message);
      }
      catch (...)
      {
      }
    }

    // ---------------------------------------------------------------------
    // Rendering
    // ---------------------------------------------------------------------

    template <typename... Ts>
    [[nodiscard]] static std::string render_arguments_(
        const LimitsSnapshot &limits,
        Ts &&...values)
    {
      std::string out;
      out.reserve(std::min<std::size_t>(limits.max_record_size, 256));

      bool first = true;
      auto append = [&](auto &&value)
      {
        if (!first)
          append_bounded_(out, " ", limits.max_record_size);
        first = false;
        append_value_(out, std::forward<decltype(value)>(value), limits);
      };

      (append(std::forward<Ts>(values)), ...);
      truncate_record_(out, limits.max_record_size);
      return out;
    }

    template <typename T>
    static void append_value_(std::string &out,
                              T &&value,
                              const LimitsSnapshot &limits)
    {
      using Raw = std::remove_reference_t<T>;
      using U = std::remove_cvref_t<T>;

      if constexpr (std::is_same_v<U, std::string>)
      {
        append_string_value_(out, std::string_view(value), limits);
      }
      else if constexpr (std::is_same_v<U, std::string_view>)
      {
        append_string_value_(out, value, limits);
      }
      else if constexpr (
          std::is_array_v<Raw> &&
          std::is_same_v<std::remove_cv_t<std::remove_extent_t<Raw>>, char>)
      {
        append_string_value_(out, std::string_view(value), limits);
      }
      else if constexpr (
          std::is_same_v<U, const char *> || std::is_same_v<U, char *>)
      {
        append_string_value_(
            out,
            value != nullptr ? std::string_view(value) : std::string_view("null"),
            limits);
      }
      else if constexpr (std::is_same_v<U, bool>)
      {
        append_bounded_(out, value ? "true" : "false", limits.max_record_size);
      }
      else if constexpr (std::is_same_v<U, char>)
      {
        char ch[1]{value};
        append_bounded_(out, std::string_view(ch, 1), limits.max_record_size);
      }
      else if constexpr (std::is_integral_v<U> && !std::is_same_v<U, bool>)
      {
        append_integral_(out, value, limits.max_record_size);
      }
      else if constexpr (std::is_floating_point_v<U>)
      {
        append_floating_(out, value, limits.max_record_size);
      }
      else if constexpr (std::is_enum_v<U>)
      {
        using Underlying = std::underlying_type_t<U>;
        append_integral_(
            out,
            static_cast<Underlying>(value),
            limits.max_record_size);
      }
      else if constexpr (std::is_same_v<U, std::nullptr_t>)
      {
        append_bounded_(out, "null", limits.max_record_size);
      }
      else if constexpr (std::is_base_of_v<std::exception, U>)
      {
        append_string_value_(out, value.what(), limits);
      }
      else
      {
        const std::string rendered = render_complex_(value, limits);
        append_bounded_(out, rendered, limits.max_record_size);
      }
    }

    template <typename T>
    [[nodiscard]] static std::string render_complex_(
        const T &value,
        const LimitsSnapshot &limits)
    {
      using U = std::remove_cvref_t<T>;

      if constexpr (vix::traits::has_formatter_v<U> ||
                    vix::traits::has_vix_format_v<U>)
      {
        std::ostringstream stream;

        print_config print_config_value = default_config();
        print_config_value.separator = " ";
        print_config_value.end.clear();
        print_config_value.out = &stream;
        print_config_value.color = false;
        print_config_value.max_items = limits.max_items;
        print_config_value.show_type = false;
        print_config_value.compact = true;
        print_config_value.raw_strings = true;

        scoped_config guard{print_config_value};
        vix::write_to(stream, value);

        std::string rendered = stream.str();
        truncate_record_(rendered, limits.max_record_size);
        return rendered;
      }

      // Inspect is the primary complex-value renderer because it understands
      // field_map<T>, inspector<T>, ADL inspection hooks, nested containers,
      // and explicit depth limits.
      inspect_options inspect_config = default_options();
      inspect_config.max_depth = limits.max_depth;
      inspect_config.max_items = limits.max_items;
      inspect_config.show_type = false;
      inspect_config.show_meta = false;
      inspect_config.compact = true;
      inspect_config.show_address = false;

      std::string rendered = vix::inspect_to_string(value, inspect_config);

      // A formatter<T> or ADL vix_format hook belongs to the print engine.
      // If inspect cannot render any nested part, retry the entire value using
      // print so existing formatter extensions remain compatible.
      if (rendered.find("<unprintable:") != std::string::npos)
      {
        std::ostringstream stream;

        print_config print_config_value = default_config();
        print_config_value.separator = " ";
        print_config_value.end.clear();
        print_config_value.out = &stream;
        print_config_value.color = false;
        print_config_value.max_items = limits.max_items;
        print_config_value.show_type = false;
        print_config_value.compact = true;
        print_config_value.raw_strings = true;

        scoped_config guard{print_config_value};
        vix::write_to(stream, value);
        rendered = stream.str();
      }

      truncate_record_(rendered, limits.max_record_size);
      return rendered;
    }

    static void append_string_value_(std::string &out,
                                     std::string_view value,
                                     const LimitsSnapshot &limits)
    {
      const std::size_t count =
          std::min<std::size_t>(value.size(), limits.max_string_length);
      append_bounded_(out, value.substr(0, count), limits.max_record_size);

      if (count < value.size())
        append_bounded_(out, kTruncatedMarker, limits.max_record_size);
    }

    template <typename Int>
    static void append_integral_(std::string &out,
                                 Int value,
                                 std::size_t max_record_size)
    {
      static_assert(std::is_integral_v<Int>);

      char buffer[std::numeric_limits<Int>::digits10 + 4]{};
      const auto result = std::to_chars(
          std::begin(buffer),
          std::end(buffer),
          value);

      if (result.ec == std::errc{})
      {
        append_bounded_(
            out,
            std::string_view(buffer, static_cast<std::size_t>(result.ptr - buffer)),
            max_record_size);
      }
      else
      {
        append_bounded_(out, "0", max_record_size);
      }
    }

    template <typename Float>
    static void append_floating_(std::string &out,
                                 Float value,
                                 std::size_t max_record_size)
    {
      char buffer[96]{};
      int size = 0;

      if constexpr (std::is_same_v<Float, long double>)
      {
        size = std::snprintf(buffer, sizeof(buffer), "%.18Lg", value);
      }
      else
      {
        size = std::snprintf(
            buffer,
            sizeof(buffer),
            "%.17g",
            static_cast<double>(value));
      }

      if (size <= 0)
      {
        append_bounded_(out, "0", max_record_size);
        return;
      }

      const std::size_t safe_size = std::min<std::size_t>(
          static_cast<std::size_t>(size),
          sizeof(buffer) - 1);
      append_bounded_(
          out,
          std::string_view(buffer, safe_size),
          max_record_size);
    }

    static void append_bounded_(std::string &out,
                                std::string_view value,
                                std::size_t max_record_size)
    {
      if (max_record_size == 0 || out.size() >= max_record_size)
        return;

      const std::size_t available = max_record_size - out.size();
      const std::size_t count = std::min<std::size_t>(available, value.size());
      out.append(value.data(), count);
    }

    static void truncate_record_(std::string &value,
                                 std::size_t max_record_size)
    {
      if (value.size() <= max_record_size)
        return;

      if (max_record_size <= kTruncatedMarker.size())
      {
        value.resize(max_record_size);
        return;
      }

      value.resize(max_record_size - kTruncatedMarker.size());
      value.append(kTruncatedMarker.data(), kTruncatedMarker.size());
    }

    static void clamp_inspect_options_(inspect_options &options,
                                       const LimitsSnapshot &limits) noexcept
    {
      if (options.max_depth <= 0 || options.max_depth > limits.max_depth)
        options.max_depth = limits.max_depth;

      if (options.max_items == 0 || options.max_items > limits.max_items)
        options.max_items = limits.max_items;

      options.out = nullptr; // inspect_to_string owns its destination stream.
    }

    // ---------------------------------------------------------------------
    // Structured event preparation
    // ---------------------------------------------------------------------

    template <typename Field>
    [[nodiscard]] static auto prepare_field_(
        const Field &field_value,
        const LimitsSnapshot &limits)
    {
      auto rendered = prepare_event_value_(field_value.get(), limits);
      using Rendered = decltype(rendered);

      std::string key{field_value.key_view()};
      if (key.empty())
        key = "field";

      return PreparedField<Rendered>{
          std::move(key),
          std::move(rendered)};
    }

    template <typename T>
    [[nodiscard]] static auto prepare_event_value_(
        const T &value,
        const LimitsSnapshot &limits)
    {
      using U = std::remove_cvref_t<T>;

      if constexpr (std::is_same_v<U, bool>)
      {
        return value;
      }
      else if constexpr (std::is_integral_v<U> && !std::is_same_v<U, bool>)
      {
        return value;
      }
      else if constexpr (std::is_floating_point_v<U>)
      {
        return value;
      }
      else if constexpr (std::is_enum_v<U>)
      {
        return static_cast<std::underlying_type_t<U>>(value);
      }
      else if constexpr (std::is_same_v<U, std::string>)
      {
        std::string out = value.substr(0, limits.max_string_length);
        if (out.size() < value.size())
          append_bounded_(out, kTruncatedMarker, limits.max_record_size);
        truncate_record_(out, limits.max_record_size);
        return out;
      }
      else if constexpr (std::is_same_v<U, std::string_view>)
      {
        std::string out{
            value.substr(0, std::min(value.size(), limits.max_string_length))};
        if (out.size() < value.size())
          append_bounded_(out, kTruncatedMarker, limits.max_record_size);
        truncate_record_(out, limits.max_record_size);
        return out;
      }
      else if constexpr (
          std::is_array_v<U> &&
          std::is_same_v<std::remove_cv_t<std::remove_extent_t<U>>, char>)
      {
        const std::string_view view{value};
        std::string out{
            view.substr(0, std::min(view.size(), limits.max_string_length))};
        if (out.size() < view.size())
          append_bounded_(out, kTruncatedMarker, limits.max_record_size);
        truncate_record_(out, limits.max_record_size);
        return out;
      }
      else if constexpr (
          std::is_same_v<U, const char *> || std::is_same_v<U, char *>)
      {
        const std::string_view view =
            value != nullptr ? std::string_view(value) : std::string_view("null");
        std::string out{
            view.substr(0, std::min(view.size(), limits.max_string_length))};
        if (out.size() < view.size())
          append_bounded_(out, kTruncatedMarker, limits.max_record_size);
        truncate_record_(out, limits.max_record_size);
        return out;
      }
      else
      {
        return render_complex_(value, limits);
      }
    }

    template <typename Tuple, std::size_t... Indices>
    void submit_prepared_event_(Level level_value,
                                const std::string &name,
                                const Tuple &prepared,
                                std::index_sequence<Indices...>) noexcept
    {
      try
      {
        if constexpr (sizeof...(Indices) == 0)
        {
          vix::log::global().logf(to_log_level_(level_value), name);
        }
        else
        {
          auto flattened = std::tuple_cat(
              std::make_tuple(
                  std::get<Indices>(prepared).key.c_str(),
                  std::get<Indices>(prepared).value)...);

          std::apply(
              [&](const auto &...arguments)
              {
                vix::log::global().logf(
                    to_log_level_(level_value),
                    name,
                    arguments...);
              },
              flattened);
        }
      }
      catch (...)
      {
      }
    }

    // ---------------------------------------------------------------------
    // Rate limiting
    // ---------------------------------------------------------------------

    [[nodiscard]] static std::uint64_t now_epoch_second_() noexcept
    {
      using namespace std::chrono;
      return static_cast<std::uint64_t>(
          duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
    }

    [[nodiscard]] RateDecision rate_decision_() noexcept
    {
      if (!rate_limit_enabled_.load(std::memory_order_acquire))
        return RateDecision{};

      const std::uint32_t maximum =
          rate_limit_max_.load(std::memory_order_acquire);
      if (maximum == 0)
        return RateDecision{};

      const std::uint64_t now = now_epoch_second_();
      std::uint64_t epoch = rate_epoch_sec_.load(std::memory_order_relaxed);
      std::uint32_t report = 0;

      if (epoch != now)
      {
        if (rate_epoch_sec_.compare_exchange_strong(
                epoch,
                now,
                std::memory_order_acq_rel,
                std::memory_order_relaxed))
        {
          rate_count_.store(0, std::memory_order_relaxed);
          report = rate_suppressed_.exchange(0, std::memory_order_acq_rel);
        }
      }

      const std::uint32_t count =
          rate_count_.fetch_add(1, std::memory_order_relaxed) + 1;

      if (count <= maximum)
        return RateDecision{true, report};

      rate_suppressed_.fetch_add(1, std::memory_order_relaxed);
      return RateDecision{false, report};
    }

    void report_suppressed_(std::uint32_t count) noexcept
    {
      try
      {
        std::string message{"(console) suppressed "};
        append_integral_(message, count, 256);
        message += " low-severity records (rate limit)";
        submit_(Level::Warn, message);
      }
      catch (...)
      {
      }
    }

    // ---------------------------------------------------------------------
    // Level, environment, and configuration helpers
    // ---------------------------------------------------------------------

    [[nodiscard]] bool enabled_(Level message_level) const noexcept
    {
      if (message_level == Level::Off)
        return false;

      const Level configured = level_.load(std::memory_order_acquire);
      if (configured == Level::Off)
        return false;

      return static_cast<std::uint8_t>(message_level) >=
             static_cast<std::uint8_t>(configured);
    }

    [[nodiscard]] static bool is_low_severity_(Level level_value) noexcept
    {
      return level_value == Level::Trace ||
             level_value == Level::Debug ||
             level_value == Level::Info;
    }

    [[nodiscard]] LimitsSnapshot limits_snapshot_() const noexcept
    {
      return LimitsSnapshot{
          max_depth_.load(std::memory_order_acquire),
          max_items_.load(std::memory_order_acquire),
          max_string_length_.load(std::memory_order_acquire),
          max_record_size_.load(std::memory_order_acquire)};
    }

    [[nodiscard]] static vix::log::LogLevel to_log_level_(Level level_value) noexcept
    {
      switch (level_value)
      {
      case Level::Trace:
        return vix::log::LogLevel::Trace;
      case Level::Debug:
        return vix::log::LogLevel::Debug;
      case Level::Info:
        return vix::log::LogLevel::Info;
      case Level::Warn:
        return vix::log::LogLevel::Warn;
      case Level::Error:
        return vix::log::LogLevel::Error;
      case Level::Critical:
        return vix::log::LogLevel::Critical;
      case Level::Off:
        return vix::log::LogLevel::Off;
      }
      return vix::log::LogLevel::Info;
    }

    [[nodiscard]] static Level from_log_level_(vix::log::LogLevel level_value) noexcept
    {
      switch (level_value)
      {
      case vix::log::LogLevel::Trace:
        return Level::Trace;
      case vix::log::LogLevel::Debug:
        return Level::Debug;
      case vix::log::LogLevel::Info:
        return Level::Info;
      case vix::log::LogLevel::Warn:
        return Level::Warn;
      case vix::log::LogLevel::Error:
        return Level::Error;
      case vix::log::LogLevel::Critical:
        return Level::Critical;
      case vix::log::LogLevel::Off:
        return Level::Off;
      }
      return Level::Info;
    }

    [[nodiscard]] static bool iequals_(std::string_view lhs,
                                       std::string_view rhs) noexcept
    {
      if (lhs.size() != rhs.size())
        return false;

      for (std::size_t index = 0; index < lhs.size(); ++index)
      {
        const unsigned char left = static_cast<unsigned char>(lhs[index]);
        const unsigned char right = static_cast<unsigned char>(rhs[index]);

        const unsigned char lower_left =
            (left >= 'A' && left <= 'Z')
                ? static_cast<unsigned char>(left + ('a' - 'A'))
                : left;
        const unsigned char lower_right =
            (right >= 'A' && right <= 'Z')
                ? static_cast<unsigned char>(right + ('a' - 'A'))
                : right;

        if (lower_left != lower_right)
          return false;
      }

      return true;
    }

    [[nodiscard]] static std::size_t parse_size_env_(
        const char *name,
        std::size_t fallback) noexcept
    {
      const char *raw = vix::utils::vix_getenv(name);
      if (raw == nullptr || *raw == '\0')
        return fallback;

      char *end = nullptr;
      const unsigned long long value = std::strtoull(raw, &end, 10);
      if (end == raw || (end != nullptr && *end != '\0') || value == 0)
        return fallback;

      if (value > static_cast<unsigned long long>(
                      std::numeric_limits<std::size_t>::max()))
        return fallback;

      return static_cast<std::size_t>(value);
    }

    void initialize_from_backend_and_env_() noexcept
    {
      try
      {
        level_.store(
            from_log_level_(vix::log::level()),
            std::memory_order_relaxed);

        if (const char *raw = vix::utils::vix_getenv("VIX_LOG_FORMAT");
            raw != nullptr && *raw != '\0')
        {
          format_.store(
              vix::log::parse_format(raw),
              std::memory_order_relaxed);
        }

        Limits configured_limits = limits();
        configured_limits.max_depth = static_cast<int>(std::min<std::size_t>(
            parse_size_env_(
                "VIX_CONSOLE_MAX_DEPTH",
                static_cast<std::size_t>(configured_limits.max_depth)),
            static_cast<std::size_t>(std::numeric_limits<int>::max())));
        configured_limits.max_items = parse_size_env_(
            "VIX_CONSOLE_MAX_ITEMS",
            configured_limits.max_items);
        configured_limits.max_string_length = parse_size_env_(
            "VIX_CONSOLE_MAX_STRING_LENGTH",
            configured_limits.max_string_length);
        configured_limits.max_record_size = parse_size_env_(
            "VIX_CONSOLE_MAX_RECORD_SIZE",
            configured_limits.max_record_size);
        set_limits(configured_limits);

        const std::size_t rate = parse_size_env_("VIX_CONSOLE_RATE_LIMIT", 0);
        if (rate > 0)
        {
          set_rate_limit(RateLimit{
              true,
              static_cast<std::uint32_t>(std::min<std::size_t>(
                  rate,
                  std::numeric_limits<std::uint32_t>::max()))});
        }

        if (const char *raw = vix::utils::vix_getenv("VIX_CONSOLE_LEVEL");
            raw != nullptr && *raw != '\0')
        {
          set_level(parse_level(raw));
        }
      }
      catch (...)
      {
        // Keep safe defaults when initialization fails.
      }
    }
  };

  /**
   * @brief Global JavaScript-like console instance.
   */
  inline Console console;

} // namespace vix

#endif // VIX_CONSOLE_CLASS_HPP
