# Vix console examples

This directory is a practical tour of `<vix/console.hpp>`.

Recommended reading order:

1. `01_basic_levels.cpp`: JavaScript-like logging calls and level filtering.
2. `02_formats.cpp`: KV, JSON, and pretty JSON output.
3. `03_types_and_collections.cpp`: primitives, STL values, paths, durations, smart pointers.
4. `04_dir_inspection.cpp`: deep inspection with `console.dir()`.
5. `05_context.cpp`: request/module/custom context fields.
6. `06_structured_events.cpp`: typed structured events with `Console::field()`.
7. `07_field_lifetime.cpp`: lvalue field refs vs owned rvalue fields.
8. `08_limits_and_truncation.cpp`: depth, item, string, and record limits.
9. `09_rate_limiting.cpp`: low-severity rate limiting.
10. `10_custom_extensions.cpp`: `vix::formatter`, ADL `vix_format`, `vix::inspector`, and `vix::field_map`.
11. `11_async_mode.cpp`: temporarily enabling async logging.
12. `12_environment.cpp`: constructing a local console after environment setup.
13. `13_local_console.cpp`: using a separate `vix::Console` instance.

Build these examples from the module build with:

```bash
cmake -S modules/io -B modules/io/build-ninja -G Ninja -DVIX_IO_BUILD_EXAMPLES=ON
cmake --build modules/io/build-ninja --target vix_io_console_01_basic_levels
```

Or from the umbrella build:

```bash
cmake -S . -B build-ninja -G Ninja -DVIX_IO_BUILD_EXAMPLES=ON
cmake --build build-ninja --target vix_io_console_06_structured_events
```
