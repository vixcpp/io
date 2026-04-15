#include <cassert>
#include <sstream>
#include <string>

#include <vix/io/Copy.hpp>
#include <vix/io/Input.hpp>
#include <vix/io/IoOptions.hpp>
#include <vix/io/Output.hpp>

namespace
{
  void test_copy_transfers_all_data()
  {
    std::istringstream in_stream("hello world");
    std::ostringstream out_stream;

    vix::io::Input input(in_stream);
    vix::io::Output output(out_stream);

    auto result = vix::io::copy(input, output);

    assert(result);
    assert(result.value() == 11);
    assert(out_stream.str() == "hello world");
  }

  void test_copy_with_small_chunk_size()
  {
    std::istringstream in_stream("abcdef");
    std::ostringstream out_stream;

    vix::io::Input input(in_stream);
    vix::io::Output output(out_stream);

    vix::io::IoOptions options;
    options.chunk_size = 2;

    auto result = vix::io::copy(input, output, options);

    assert(result);
    assert(result.value() == 6);
    assert(out_stream.str() == "abcdef");
  }

  void test_copy_empty_stream()
  {
    std::istringstream in_stream("");
    std::ostringstream out_stream;

    vix::io::Input input(in_stream);
    vix::io::Output output(out_stream);

    auto result = vix::io::copy(input, output);

    assert(result);
    assert(result.value() == 0);
    assert(out_stream.str().empty());
  }
}

int main()
{
  test_copy_transfers_all_data();
  test_copy_with_small_chunk_size();
  test_copy_empty_stream();
  return 0;
}
