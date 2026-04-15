#include <cassert>
#include <sstream>
#include <string>

#include <vix/io/Input.hpp>
#include <vix/io/IoOptions.hpp>
#include <vix/io/ReadLine.hpp>

namespace
{
  void test_read_line_reads_first_line()
  {
    std::istringstream stream("hello\nworld\n");
    vix::io::Input input(stream);

    auto result = vix::io::read_line(input);

    assert(result);
    assert(result.value() == "hello");
  }

  void test_read_line_reads_empty_line()
  {
    std::istringstream stream("\nsecond\n");
    vix::io::Input input(stream);

    auto result = vix::io::read_line(input);

    assert(result);
    assert(result.value().empty());
  }

  void test_read_line_keep_newline()
  {
    std::istringstream stream("hello\n");
    vix::io::Input input(stream);

    vix::io::IoOptions options;
    options.keep_newline = true;

    auto result = vix::io::read_line(input, options);

    assert(result);
    assert(result.value() == "hello\n");
  }

  void test_read_line_eof_returns_empty_string()
  {
    std::istringstream stream("");
    vix::io::Input input(stream);

    auto result = vix::io::read_line(input);

    assert(result);
    assert(result.value().empty());
  }
}

int main()
{
  test_read_line_reads_first_line();
  test_read_line_reads_empty_line();
  test_read_line_keep_newline();
  test_read_line_eof_returns_empty_string();
  return 0;
}
