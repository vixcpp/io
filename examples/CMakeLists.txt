function(vix_io_add_example example_name example_source)
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

vix_io_add_example(vix_io_example_basic basic.cpp)
vix_io_add_example(vix_io_example_advanced advanced.cpp)
vix_io_add_example(vix_io_example_buffer buffer.cpp)
vix_io_add_example(vix_io_example_copy_stream copy_stream.cpp)
vix_io_add_example(vix_io_example_read_all read_all.cpp)
vix_io_add_example(vix_io_example_stdin stdin.cpp)
vix_io_add_example(vix_io_example_stdout stdout.cpp)

add_subdirectory(console)
