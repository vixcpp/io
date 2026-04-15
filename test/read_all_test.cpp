#include <cassert>
#include <sstream>
#include <string>

#include <vix/io/Input.hpp>
#include <vix/io/IoOptions.hpp>
#include <vix/io/ReadAll.hpp>

namespace
{
  void test_read_all_reads_entire_stream()
  {
    std::istringstream stream("hello world");
    vix::io::Input input(stream);

    auto result = vix::io::read_all(input);

    assert(result);
    const std::string text(result.value().begin(), result.value().end());
    assert(text == "hello world");
  }

  void test_read_all_with_small_chunk_size()
  {
    std::istringstream stream("abcdef");
    vix::io::Input input(stream);

    vix::io::IoOptions options;
    options.chunk_size = 2;

    auto result = vix::io::read_all(input, options);

    assert(result);
    const std::string text(result.value().begin(), result.value().end());
    assert(text == "abcdef");
  }

  void test_read_all_empty_stream()
  {
    std::istringstream stream("");
    vix::io::Input input(stream);

    auto result = vix::io::read_all(input);

    assert(result);
    assert(result.value().empty());
  }
}

int main()
{
  test_read_all_reads_entire_stream();
  test_read_all_with_small_chunk_size();
  test_read_all_empty_stream();
  return 0;
}
