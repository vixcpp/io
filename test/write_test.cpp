#include <cassert>
#include <sstream>
#include <string>

#include <vix/io/Flush.hpp>
#include <vix/io/IoOptions.hpp>
#include <vix/io/Output.hpp>
#include <vix/io/Write.hpp>
#include <vix/io/WriteLine.hpp>

namespace
{
  void test_write_text_writes_to_stream()
  {
    std::ostringstream stream;
    vix::io::Output output(stream);

    auto result = vix::io::write(output, "Hello");

    assert(result);
    assert(result.value() == 5);
    assert(stream.str() == "Hello");
  }

  void test_write_bytes_writes_to_stream()
  {
    std::ostringstream stream;
    vix::io::Output output(stream);

    vix::io::Bytes bytes{65, 66, 67};

    auto result = vix::io::write(output, bytes);

    assert(result);
    assert(result.value() == 3);
    assert(stream.str() == "ABC");
  }

  void test_write_line_appends_newline()
  {
    std::ostringstream stream;
    vix::io::Output output(stream);

    auto result = vix::io::write_line(output, "Hello");

    assert(result);
    assert(result.value() == 6);
    assert(stream.str() == "Hello\n");
  }

  void test_write_line_with_crlf()
  {
    std::ostringstream stream;
    vix::io::Output output(stream);

    vix::io::IoOptions options;
    options.newline_mode = vix::io::NewlineMode::CRLF;

    auto result = vix::io::write_line(output, "Hello", options);

    assert(result);
    assert(result.value() == 7);
    assert(stream.str() == "Hello\r\n");
  }

  void test_flush_succeeds()
  {
    std::ostringstream stream;
    vix::io::Output output(stream);

    auto result = vix::io::flush(output);

    assert(result);
    assert(result.value() == true);
  }
}

int main()
{
  test_write_text_writes_to_stream();
  test_write_bytes_writes_to_stream();
  test_write_line_appends_newline();
  test_write_line_with_crlf();
  test_flush_succeeds();
  return 0;
}
