add_executable(vix_io_buffer_test buffer_test.cpp)
target_compile_features(vix_io_buffer_test PRIVATE cxx_std_20)
target_link_libraries(vix_io_buffer_test PRIVATE vix::io)
add_test(NAME vix_io_buffer_test COMMAND vix_io_buffer_test)

add_executable(vix_io_copy_test copy_test.cpp)
target_compile_features(vix_io_copy_test PRIVATE cxx_std_20)
target_link_libraries(vix_io_copy_test PRIVATE vix::io)
add_test(NAME vix_io_copy_test COMMAND vix_io_copy_test)

add_executable(vix_io_read_all_test read_all_test.cpp)
target_compile_features(vix_io_read_all_test PRIVATE cxx_std_20)
target_link_libraries(vix_io_read_all_test PRIVATE vix::io)
add_test(NAME vix_io_read_all_test COMMAND vix_io_read_all_test)

add_executable(vix_io_read_line_test read_line_test.cpp)
target_compile_features(vix_io_read_line_test PRIVATE cxx_std_20)
target_link_libraries(vix_io_read_line_test PRIVATE vix::io)
add_test(NAME vix_io_read_line_test COMMAND vix_io_read_line_test)

add_executable(vix_io_write_test write_test.cpp)
target_compile_features(vix_io_write_test PRIVATE cxx_std_20)
target_link_libraries(vix_io_write_test PRIVATE vix::io)
add_test(NAME vix_io_write_test COMMAND vix_io_write_test)
