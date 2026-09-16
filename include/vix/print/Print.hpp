/** @file Print.hpp Configurable Vix rendering. */
#ifndef VIX_PRINT_CLASS_HPP
#define VIX_PRINT_CLASS_HPP

#include <algorithm>
#include <any>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
#include <expected>
#define VIX_PRINT_HAS_EXPECTED 1
#else
#define VIX_PRINT_HAS_EXPECTED 0
#endif

#include <vix/print/Formatter.hpp>

namespace vix
{
  struct print_config
  {
    std::string separator = " ";
    std::string end = "\n";
    std::ostream *out = &std::cout;
    bool color = false;
    std::size_t max_items = 256;
    bool show_type = false;
    bool compact = false;
    std::string indent_str = "  ";
    bool raw_strings = true;
  };

  struct options
  {
    std::string sep = " ";
    std::string end = "\n";
    std::ostream *file = &std::cout;
    bool flush = false;
    bool raw_strings = true;
    std::size_t max_items = 256;
    bool compact = true;
    std::string indent = "  ";
    bool show_type = false;
    bool color = false;
  };

  inline print_config &default_config() noexcept
  {
    static thread_local print_config config;
    return config;
  }

  [[nodiscard]] inline print_config to_print_config(const options &value)
  {
    print_config config = default_config();
    config.separator = value.sep;
    config.end = value.end;
    config.out = value.file;
    config.raw_strings = value.raw_strings;
    config.max_items = value.max_items;
    config.compact = value.compact;
    config.indent_str = value.indent;
    config.show_type = value.show_type;
    config.color = value.color;
    return config;
  }

  class print_context;
  namespace detail
  {
    template <typename T>
    void write(print_context &, const T &);
  }

  /** Per-render state. Context ADL formatters can recurse with ctx.write(value). */
  class print_context
  {
  public:
    std::ostream &out;
    const print_config &config;
    template <typename T>
    void write(const T &value) { detail::write(*this, value); }
  };

  namespace detail
  {
    template <typename T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    template <typename T, typename = void>
    struct has_formatter : std::false_type
    {
    };
    template <typename T>
    struct has_formatter<T, std::void_t<decltype(formatter<T>::format(std::declval<std::ostream &>(), std::declval<const T &>()))>> : std::true_type
    {
    };
    template <typename T>
    inline constexpr bool has_formatter_v = has_formatter<remove_cvref_t<T>>::value;

    template <typename T>
    concept context_adl_formattable = requires(print_context &ctx, const T &value) { vix_format(ctx, value); };
    template <typename T>
    concept legacy_adl_formattable = requires(std::ostream &out, const T &value) { vix_format(out, value); };
    template <typename T>
    concept streamable = requires(std::ostream &out, const T &value) { out << value; };
    template <typename T>
    concept range_like = requires(const T &value) { std::begin(value); std::end(value); };
    template <typename T>
    concept tuple_like = requires { std::tuple_size<remove_cvref_t<T>>::value; };

    template <typename T>
    struct is_optional : std::false_type
    {
    };
    template <typename T>
    struct is_optional<std::optional<T>> : std::true_type
    {
    };
    template <typename T>
    struct is_variant : std::false_type
    {
    };
    template <typename... T>
    struct is_variant<std::variant<T...>> : std::true_type
    {
    };
    template <typename T>
    struct is_unique_ptr : std::false_type
    {
    };
    template <typename T, typename D>
    struct is_unique_ptr<std::unique_ptr<T, D>> : std::true_type
    {
    };
    template <typename T>
    struct is_shared_ptr : std::false_type
    {
    };
    template <typename T>
    struct is_shared_ptr<std::shared_ptr<T>> : std::true_type
    {
    };
    template <typename T>
    struct is_weak_ptr : std::false_type
    {
    };
    template <typename T>
    struct is_weak_ptr<std::weak_ptr<T>> : std::true_type
    {
    };
    template <typename T>
    struct is_reference_wrapper : std::false_type
    {
    };
    template <typename T>
    struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type
    {
    };
    template <typename T>
    struct is_stack : std::false_type
    {
    };
    template <typename V, typename C>
    struct is_stack<std::stack<V, C>> : std::true_type
    {
    };
    template <typename T>
    struct is_queue : std::false_type
    {
    };
    template <typename V, typename C>
    struct is_queue<std::queue<V, C>> : std::true_type
    {
    };
    template <typename T>
    struct is_priority_queue : std::false_type
    {
    };
    template <typename V, typename C, typename P>
    struct is_priority_queue<std::priority_queue<V, C, P>> : std::true_type
    {
    };
    template <typename T>
    struct is_map : std::false_type
    {
    };
    template <typename K, typename V, typename C, typename A>
    struct is_map<std::map<K, V, C, A>> : std::true_type
    {
    };
    template <typename K, typename V, typename C, typename A>
    struct is_map<std::multimap<K, V, C, A>> : std::true_type
    {
    };
    template <typename K, typename V, typename H, typename E, typename A>
    struct is_map<std::unordered_map<K, V, H, E, A>> : std::true_type
    {
    };
    template <typename K, typename V, typename H, typename E, typename A>
    struct is_map<std::unordered_multimap<K, V, H, E, A>> : std::true_type
    {
    };
    template <typename T>
    struct is_pair : std::false_type
    {
    };
    template <typename A, typename B>
    struct is_pair<std::pair<A, B>> : std::true_type
    {
    };
#if VIX_PRINT_HAS_EXPECTED
    template <typename T>
    struct is_expected : std::false_type
    {
    };
    template <typename V, typename E>
    struct is_expected<std::expected<V, E>> : std::true_type
    {
    };
#endif

    template <typename T>
    inline constexpr bool narrow_string = std::is_same_v<remove_cvref_t<T>, std::string> ||
                                          std::is_same_v<remove_cvref_t<T>, std::string_view> || std::is_same_v<remove_cvref_t<T>, char *> ||
                                          std::is_same_v<remove_cvref_t<T>, const char *> || (std::is_array_v<std::remove_reference_t<T>> && std::is_same_v<std::remove_cv_t<std::remove_extent_t<std::remove_reference_t<T>>>, char>);
    template <typename T>
    inline constexpr bool wide_string = std::is_same_v<remove_cvref_t<T>, std::wstring> ||
                                        std::is_same_v<remove_cvref_t<T>, std::wstring_view> || std::is_same_v<remove_cvref_t<T>, wchar_t *> ||
                                        std::is_same_v<remove_cvref_t<T>, const wchar_t *> || (std::is_array_v<std::remove_reference_t<T>> && std::is_same_v<std::remove_cv_t<std::remove_extent_t<std::remove_reference_t<T>>>, wchar_t>);

    template <typename T>
    void write_tuple(print_context &ctx, const T &value)
    {
      ctx.out << '(';
      [&]<std::size_t... I>(std::index_sequence<I...>)
      { ((ctx.out << (I == 0 ? "" : ", "), ctx.write(std::get<I>(value))), ...); }(std::make_index_sequence<std::tuple_size_v<remove_cvref_t<T>>>{});
      ctx.out << ')';
    }

    template <bool IsMap, typename Range>
    void write_range(print_context &ctx, const Range &range)
    {
      ctx.out << (IsMap ? '{' : '[');
      std::size_t count = 0;
      bool first = true;
      for (const auto &element : range)
      {
        if (count++ >= ctx.config.max_items)
        {
          ctx.out << ", ...";
          break;
        }
        if (!first)
          ctx.out << ", ";
        first = false;
        if constexpr (IsMap)
        {
          ctx.write(element.first);
          ctx.out << " => ";
          ctx.write(element.second);
        }
        else
          ctx.write(element);
      }
      ctx.out << (IsMap ? '}' : ']');
    }

    template <typename Adapter>
    void write_adapter(print_context &ctx, Adapter copy, std::string_view name)
    {
      ctx.out << name << '[';
      std::size_t count = 0;
      bool first = true;
      while (!copy.empty())
      {
        if (count++ >= ctx.config.max_items)
        {
          ctx.out << ", ...";
          break;
        }
        if (!first)
          ctx.out << ", ";
        first = false;
        if constexpr (is_stack<remove_cvref_t<Adapter>>::value || is_priority_queue<remove_cvref_t<Adapter>>::value)
          ctx.write(copy.top());
        else
          ctx.write(copy.front());
        copy.pop();
      }
      ctx.out << ']';
    }

    template <typename T>
    void write(print_context &ctx, const T &value)
    {
      using U = remove_cvref_t<T>;
      if constexpr (has_formatter_v<U>)
        formatter<U>::format(ctx.out, value);
      else if constexpr (context_adl_formattable<U>)
        vix_format(ctx, value);
      else if constexpr (legacy_adl_formattable<U>)
        vix_format(ctx.out, value);
      else if constexpr (std::is_same_v<U, std::nullptr_t>)
        ctx.out << "nullptr";
      else if constexpr (std::is_same_v<U, bool>)
        ctx.out << (value ? "true" : "false");
      else if constexpr (std::is_same_v<U, char> || std::is_same_v<U, signed char> || std::is_same_v<U, unsigned char>)
        ctx.out << '\'' << static_cast<char>(value) << '\'';
      else if constexpr (std::is_same_v<U, wchar_t>)
        ctx.out << "L'\\u" << std::hex << std::uppercase << static_cast<unsigned>(value) << std::dec << '\'';
      else if constexpr (std::is_same_v<U, char8_t>)
        ctx.out << "u8'" << static_cast<char>(value) << '\'';
      else if constexpr (std::is_same_v<U, char16_t>)
        ctx.out << "u'\\u" << std::hex << std::uppercase << static_cast<unsigned>(value) << std::dec << '\'';
      else if constexpr (std::is_same_v<U, char32_t>)
        ctx.out << "U'\\U" << std::hex << std::uppercase << static_cast<unsigned long>(value) << std::dec << '\'';
      else if constexpr (narrow_string<T>)
      {
        if constexpr (std::is_pointer_v<U>)
          if (value == nullptr)
          {
            ctx.out << "nullptr";
            return;
          }
        if (ctx.config.raw_strings)
          ctx.out << value;
        else
          ctx.out << '"' << value << '"';
      }
      else if constexpr (wide_string<T>)
      {
        if constexpr (std::is_pointer_v<U>)
          if (value == nullptr)
          {
            ctx.out << "nullptr";
            return;
          }
        ctx.out << "L\"";
        for (wchar_t character : std::wstring_view{value})
        {
          if (character < 0x80)
            ctx.out << static_cast<char>(character);
          else
            ctx.out << "\\u" << std::hex << static_cast<unsigned>(character) << std::dec;
        }
        ctx.out << '"';
      }
      else if constexpr (std::is_same_v<U, std::any>)
        ctx.out << (value.has_value() ? "any(value)" : "any(empty)");
      else if constexpr (is_optional<U>::value)
      {
        if (value)
        {
          ctx.out << "Some(";
          ctx.write(*value);
          ctx.out << ')';
        }
        else
          ctx.out << "None";
      }
#if VIX_PRINT_HAS_EXPECTED
      else if constexpr (is_expected<U>::value)
      {
        if (value)
        {
          ctx.out << "Ok(";
          ctx.write(*value);
          ctx.out << ')';
        }
        else
        {
          ctx.out << "Err(";
          ctx.write(value.error());
          ctx.out << ')';
        }
      }
#endif
      else if constexpr (is_variant<U>::value)
      {
        ctx.out << "variant<";
        std::visit([&ctx](const auto &active)
                   { ctx.write(active); }, value);
        ctx.out << '>';
      }
      else if constexpr (is_reference_wrapper<U>::value)
      {
        ctx.out << "ref(";
        ctx.write(value.get());
        ctx.out << ')';
      }
      else if constexpr (is_unique_ptr<U>::value)
      {
        if (!value)
          ctx.out << "unique_ptr(null)";
        else
        {
          ctx.out << "unique_ptr(";
          ctx.write(*value);
          ctx.out << ')';
        }
      }
      else if constexpr (is_shared_ptr<U>::value)
      {
        if (!value)
          ctx.out << "shared_ptr(null)";
        else
        {
          ctx.out << "shared_ptr[use=" << value.use_count() << "](";
          ctx.write(*value);
          ctx.out << ')';
        }
      }
      else if constexpr (is_weak_ptr<U>::value)
      {
        if (value.expired())
          ctx.out << "weak_ptr(expired)";
        else if (auto shared = value.lock())
        {
          ctx.out << "weak_ptr[use=" << shared.use_count() << "](";
          ctx.write(*shared);
          ctx.out << ')';
        }
      }
      else if constexpr (is_stack<U>::value)
        write_adapter(ctx, value, "stack");
      else if constexpr (is_queue<U>::value)
        write_adapter(ctx, value, "queue");
      else if constexpr (is_priority_queue<U>::value)
        write_adapter(ctx, value, "priority_queue");
      // A user-defined stream insertion is more intentional than generic range/tuple shape.
      else if constexpr (streamable<U>)
        ctx.out << value;
      else if constexpr (is_map<U>::value)
        write_range<true>(ctx, value);
      else if constexpr (range_like<U> && !narrow_string<U> && !wide_string<U>)
        write_range<false>(ctx, value);
      else if constexpr (is_pair<U>::value)
      {
        ctx.out << '(';
        ctx.write(value.first);
        ctx.out << ": ";
        ctx.write(value.second);
        ctx.out << ')';
      }
      else if constexpr (tuple_like<U>)
        write_tuple(ctx, value);
      else if constexpr (std::is_enum_v<U>)
        ctx.out << static_cast<std::underlying_type_t<U>>(value);
      else if constexpr (std::is_pointer_v<U>)
      {
        if (value == nullptr)
          ctx.out << "nullptr";
        else
          ctx.out << "<ptr>";
      }
      else
        ctx.out << "<unprintable>";
    }

    template <typename T>
    void write(std::ostream &out, const T &value)
    {
      print_context context{out, default_config()};
      context.write(value);
    }
    template <typename... Args>
    void print_impl(const print_config &config, const Args &...args)
    {
      print_context context{*config.out, config};
      bool first = true;
      (([&]
        { if (!first) context.out << config.separator; first = false; context.write(args); }()),
       ...);
      context.out << config.end;
    }
  }

  template <typename T>
  void write_to(std::ostream &out, const T &value, const print_config &config)
  {
    print_context context{out, config};
    context.write(value);
  }
  template <typename T>
  void write_to(std::ostream &out, const T &value) { write_to(out, value, default_config()); }
  template <typename T>
  [[nodiscard]] std::string to_string(const T &value, const print_config &config)
  {
    std::ostringstream out;
    write_to(out, value, config);
    return out.str();
  }
  template <typename T>
  [[nodiscard]] std::string to_string(const T &value) { return to_string(value, default_config()); }
  template <typename... Args>
  void print(const Args &...args) { detail::print_impl(default_config(), args...); }
  template <typename... Args>
  void print(const print_config &config, const Args &...args) { detail::print_impl(config, args...); }
  template <typename... Args>
  void print(const options &value, const Args &...args)
  {
    const auto config = to_print_config(value);
    detail::print_impl(config, args...);
    if (value.flush && value.file)
      value.file->flush();
  }
  template <typename... Args>
  void print_py(const Args &...args)
  {
    auto config = default_config();
    config.raw_strings = true;
    detail::print_impl(config, args...);
  }
  template <typename... Args>
  void print_to(std::ostream &out, const Args &...args)
  {
    auto config = default_config();
    config.out = &out;
    detail::print_impl(config, args...);
  }
  template <typename... Args>
  void eprint(const Args &...args) { print_to(std::cerr, args...); }
  template <typename... Args>
  void print_inline(const Args &...args)
  {
    auto config = default_config();
    config.end.clear();
    detail::print_impl(config, args...);
  }
  template <typename... Args>
  [[nodiscard]] std::string sprint(const Args &...args)
  {
    std::ostringstream out;
    auto config = default_config();
    config.out = &out;
    config.end.clear();
    detail::print_impl(config, args...);
    return out.str();
  }
  inline void print() { *default_config().out << default_config().end; }

  template <typename T>
  void print_named(std::string_view label, const T &value)
  {
    const auto config = default_config();
    *config.out << label << ": ";
    write_to(*config.out, value, config);
    *config.out << config.end;
  }
  inline void print_separator(std::size_t width = 60, char character = '-')
  {
    for (std::size_t i = 0; i < width; ++i)
      *default_config().out << character;
    *default_config().out << '\n';
  }
  inline void print_header(std::string_view title, std::size_t width = 60)
  {
    print_separator(width);
    *default_config().out << title << '\n';
    print_separator(width);
  }
  template <typename Range>
    requires detail::range_like<Range>
  void print_each(const Range &range, bool show_index = false)
  {
    const auto config = default_config();
    std::size_t index = 0;
    for (const auto &value : range)
    {
      if (show_index)
        *config.out << '[' << index++ << "] ";
      write_to(*config.out, value, config);
      *config.out << config.end;
    }
  }
  template <typename Map>
    requires detail::is_map<detail::remove_cvref_t<Map>>::value
  void print_table(const Map &map, std::size_t width = 20)
  {
    const auto config = default_config();
    print_separator(width * 2 + 3);
    for (const auto &[key, value] : map)
    {
      const auto key_text = to_string(key, config);
      *config.out << key_text << std::string(key_text.size() < width ? width - key_text.size() : 1, ' ') << "| " << to_string(value, config) << '\n';
    }
    print_separator(width * 2 + 3);
  }

  class scoped_config
  {
  public:
    print_config cfg;
    explicit scoped_config(print_config override_config = default_config()) : cfg(std::move(override_config)), saved_(default_config()) { default_config() = cfg; }
    ~scoped_config() { default_config() = saved_; }
    scoped_config(const scoped_config &) = delete;
    scoped_config &operator=(const scoped_config &) = delete;

  private:
    print_config saved_;
  };

  template <typename T>
  struct streamable_formatter
  {
    static void format(std::ostream &out, const T &value) { out << value; }
  };
  template <>
  struct formatter<std::monostate>
  {
    static void format(std::ostream &out, const std::monostate &) { out << "monostate"; }
  };
  template <>
  struct formatter<std::byte>
  {
    static void format(std::ostream &out, const std::byte &value) { out << "0x" << std::hex << static_cast<unsigned>(value) << std::dec; }
  };
  template <>
  struct formatter<std::error_code>
  {
    static void format(std::ostream &out, const std::error_code &value) { out << "error_code(" << value.value() << ", \"" << value.message() << "\")"; }
  };
  template <>
  struct formatter<std::error_condition>
  {
    static void format(std::ostream &out, const std::error_condition &value) { out << "error_condition(" << value.value() << ", \"" << value.message() << "\")"; }
  };
  template <typename T>
  [[nodiscard]] std::string_view rendering_path() noexcept
  {
    using U = detail::remove_cvref_t<T>;
    if constexpr (detail::has_formatter_v<U>)
      return "vix::formatter<T> specialization";
    else if constexpr (detail::context_adl_formattable<U>)
      return "ADL vix_format(print_context&, T)";
    else if constexpr (detail::legacy_adl_formattable<U>)
      return "ADL vix_format(ostream&, T)";
    else if constexpr (detail::streamable<U>)
      return "operator<< (streamable)";
    else if constexpr (detail::range_like<U>)
      return "range";
    else
      return "fallback (unprintable)";
  }
  template <typename... Ts>
  void print_rendering_paths() { (print(rendering_path<Ts>()), ...); }

  namespace pretty
  {
    struct context
    {
      std::ostream &os;
      int depth = 0;
      int max_depth = 8;
      std::size_t max_items = 32;
      std::string indent_str = "  ";
      bool color = false;
      void indent() const
      {
        for (int i = 0; i < depth; ++i)
          os << indent_str;
      }
      void newline() const { os << '\n'; }
    };
    template <typename T>
    void pwrite(context &ctx, const T &value)
    {
      print_config config;
      config.out = &ctx.os;
      config.max_items = ctx.max_items;
      config.indent_str = ctx.indent_str;
      config.color = ctx.color;
      write_to(ctx.os, value, config);
    }
  }
  template <typename T>
  void pprint(const T &value, const print_config &config = default_config())
  {
    write_to(*config.out, value, config);
    *config.out << config.end;
  }
  template <typename... Args>
  void pprint_all(const Args &...args) { (pprint(args), ...); }
  template <typename... Args>
  void print_if(bool condition, const Args &...args)
  {
    if (condition)
      print(args...);
  }
  template <typename... Args>
  void dprint(const Args &...args)
  {
#ifndef NDEBUG
    print_to(std::cerr, args...);
#else
    (void)sizeof...(args);
#endif
  }
  template <typename T>
  const T &tap(const T &value, std::string_view label = "")
  {
    if (label.empty())
      print(value);
    else
      print_named(label, value);
    return value;
  }
  template <typename T>
  void print_typed(const T &value) { print(value); }
  template <typename A, typename B>
  void print_diff(const A &left, const B &right, std::string_view left_label = "left", std::string_view right_label = "right")
  {
    print_named(left_label, left);
    print_named(right_label, right);
  }
  template <typename A, typename B>
  bool print_check(std::string_view label, const A &actual, const B &expected)
  {
    bool equal = false;
    if constexpr (requires { actual == expected; })
      equal = actual == expected;
    print(equal ? "[PASS]" : "[FAIL]", label);
    if (!equal)
    {
      print_named("expected", expected);
      print_named("got", actual);
    }
    return equal;
  }

  template <typename Range>
    requires detail::range_like<Range>
  void print_stats(const Range &range, std::string_view label = "")
  {
    using value_type = detail::remove_cvref_t<decltype(*std::begin(range))>;
    auto &out = *default_config().out;
    if (!label.empty())
      out << label << ": ";
    if (std::begin(range) == std::end(range))
    {
      out << "stats(empty)\n";
      return;
    }
    value_type min = *std::begin(range), max = min;
    double sum = 0.0;
    std::size_t count = 0;
    for (const auto &value : range)
    {
      if (value < min)
        min = value;
      if (value > max)
        max = value;
      sum += static_cast<double>(value);
      ++count;
    }
    out << "stats{n=" << count << ", min=" << min << ", max=" << max << ", sum=" << sum << ", avg=" << sum / count << "}\n";
  }
  template <typename Map>
    requires detail::is_map<detail::remove_cvref_t<Map>>::value
  void print_sorted_by_value(const Map &map)
  {
    using item = detail::remove_cvref_t<decltype(*std::begin(map))>;
    std::vector<item> items(std::begin(map), std::end(map));
    std::sort(items.begin(), items.end(), [](const auto &a, const auto &b)
              { return a.second < b.second; });
    print(items);
  }
  template <typename Range, typename Predicate>
    requires detail::range_like<Range>
  void print_where(const Range &range, Predicate &&predicate)
  {
    std::vector<detail::remove_cvref_t<decltype(*std::begin(range))>> values;
    for (const auto &value : range)
      if (predicate(value))
        values.push_back(value);
    print(values);
  }
  template <typename Range, typename Function>
    requires detail::range_like<Range>
  void print_map_fn(const Range &range, Function &&function)
  {
    std::ostringstream out;
    auto config = default_config();
    config.out = &out;
    print_context context{out, config};
    out << '[';
    bool first = true;
    std::size_t count = 0;
    for (const auto &value : range)
    {
      if (count++ >= config.max_items)
      {
        out << ", ...";
        break;
      }
      if (!first)
        out << ", ";
      first = false;
      context.write(function(value));
    }
    out << ']' << config.end;
    *default_config().out << out.str();
  }

  class string_builder
  {
  public:
    template <typename T>
    string_builder &append(const T &value)
    {
      write_to(buffer_, value, config_);
      return *this;
    }
    string_builder &append_raw(std::string_view value)
    {
      buffer_ << value;
      return *this;
    }
    string_builder &sep(std::string_view value = " ")
    {
      if (has_content_)
        buffer_ << value;
      has_content_ = true;
      return *this;
    }
    template <typename T>
    string_builder &sep_append(const T &value, std::string_view separator = " ") { return sep(separator).append(value); }
    [[nodiscard]] std::string str() const { return buffer_.str(); }
    [[nodiscard]] bool empty() const { return buffer_.str().empty(); }
    void clear()
    {
      buffer_.str({});
      buffer_.clear();
      has_content_ = false;
    }
    explicit operator std::string() const { return str(); }

  private:
    std::ostringstream buffer_;
    print_config config_{};
    bool has_content_ = false;
  };

  template <typename Range>
    requires detail::range_like<Range>
  void print_summary(const Range &range, std::size_t head = 3, std::size_t tail = 3)
  {
    using value_type = detail::remove_cvref_t<decltype(*std::begin(range))>;
    std::vector<value_type> values(std::begin(range), std::end(range));
    auto config = default_config();
    auto &out = *config.out;
    out << "range[n=" << values.size() << "](";
    const auto first_count = std::min(head, values.size());
    for (std::size_t i = 0; i < first_count; ++i)
    {
      if (i)
        out << ", ";
      write_to(out, values[i], config);
    }
    if (first_count < values.size())
    {
      const auto last_count = std::min(tail, values.size() - first_count);
      out << ", ...";
      for (std::size_t i = values.size() - last_count; i < values.size(); ++i)
      {
        out << ", ";
        write_to(out, values[i], config);
      }
    }
    out << ')' << config.end;
  }
  inline void print_boxed(std::string_view message, std::size_t min_width = 40)
  {
    auto &out = *default_config().out;
    const std::size_t width = std::max(message.size() + 4, min_width);
    out << '+' << std::string(width, '-') << "+\n|  " << message << std::string(width - message.size() - 2, ' ') << "|\n+" << std::string(width, '-') << "+\n";
  }
  template <typename T>
  void print_boxed_value(std::string_view label, const T &value) { print_boxed(sprint(label, ": ", value)); }

  namespace ansi
  {
    inline constexpr std::string_view RESET = "\033[0m", BOLD = "\033[1m", RED = "\033[31m", GREEN = "\033[32m", YELLOW = "\033[33m", BLUE = "\033[34m", MAGENTA = "\033[35m", CYAN = "\033[36m", WHITE = "\033[37m", GRAY = "\033[90m";
    [[nodiscard]] inline std::string colorize(std::string_view value, std::string_view color) { return std::string{color} + std::string{value} + std::string{RESET}; }
    template <typename... Args>
    void print_ok(const Args &...args)
    {
      *default_config().out << GREEN;
      print(args...);
      *default_config().out << RESET;
    }
    template <typename... Args>
    void print_err(const Args &...args)
    {
      std::cerr << RED;
      print_to(std::cerr, args...);
      std::cerr << RESET;
    }
    template <typename... Args>
    void print_warn(const Args &...args)
    {
      *default_config().out << YELLOW;
      print(args...);
      *default_config().out << RESET;
    }
  }
}

#ifdef VIX_ENABLE_MACROS
#define VIX_LOG(...) ::vix::print_to(std::cerr, "[", __FILE__, ":", __LINE__, "] ", __VA_ARGS__)
#define VIX_DLOG(...) ::vix::dprint("[", __FILE__, ":", __LINE__, "] ", __VA_ARGS__)
#define VIX_INSPECT(x) ::vix::print_named(#x, (x))
#define VIX_CHECK(a, b) ::vix::print_check(#a " == " #b, (a), (b))
#endif

#endif
