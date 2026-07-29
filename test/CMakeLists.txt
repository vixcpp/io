function(vix_io_add_test test_name test_source)
  add_executable(${test_name}
    ${test_source}
  )

  target_compile_features(${test_name}
    PRIVATE
      cxx_std_20
  )

  target_link_libraries(${test_name}
    PRIVATE
      vix::io
  )

  if (VIX_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(${test_name}
      PRIVATE
        -fno-omit-frame-pointer
        -fsanitize=address,undefined
    )

    target_link_options(${test_name}
      PRIVATE
        -fsanitize=address,undefined
    )
  endif()

  add_test(
    NAME ${test_name}
    COMMAND ${test_name}
  )
endfunction()

vix_io_add_test(vix_io_buffer_test buffer_test.cpp)
vix_io_add_test(vix_io_copy_test copy_test.cpp)
vix_io_add_test(vix_io_console_test ConsoleTests.cpp)
vix_io_add_test(vix_io_read_all_test read_all_test.cpp)
vix_io_add_test(vix_io_read_line_test read_line_test.cpp)
vix_io_add_test(vix_io_write_test write_test.cpp)
