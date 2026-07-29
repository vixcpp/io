function(vix_io_add_console_example example_name example_source)
  add_executable(${example_name}
    ${example_source}
  )

  target_compile_features(${example_name}
    PRIVATE
      cxx_std_20
  )

  target_link_libraries(${example_name}
    PRIVATE
      vix::io
  )
endfunction()

vix_io_add_console_example(vix_io_console_01_basic_levels 01_basic_levels.cpp)
vix_io_add_console_example(vix_io_console_02_formats 02_formats.cpp)
vix_io_add_console_example(vix_io_console_03_types_and_collections 03_types_and_collections.cpp)
vix_io_add_console_example(vix_io_console_04_dir_inspection 04_dir_inspection.cpp)
vix_io_add_console_example(vix_io_console_05_context 05_context.cpp)
vix_io_add_console_example(vix_io_console_06_structured_events 06_structured_events.cpp)
vix_io_add_console_example(vix_io_console_07_field_lifetime 07_field_lifetime.cpp)
vix_io_add_console_example(vix_io_console_08_limits_and_truncation 08_limits_and_truncation.cpp)
vix_io_add_console_example(vix_io_console_09_rate_limiting 09_rate_limiting.cpp)
vix_io_add_console_example(vix_io_console_10_custom_extensions 10_custom_extensions.cpp)
vix_io_add_console_example(vix_io_console_11_async_mode 11_async_mode.cpp)
vix_io_add_console_example(vix_io_console_12_environment 12_environment.cpp)
vix_io_add_console_example(vix_io_console_13_local_console 13_local_console.cpp)
