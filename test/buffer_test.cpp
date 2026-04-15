#include <cassert>
#include <string>
#include <vector>

#include <vix/io/Buffer.hpp>

namespace
{
  void test_buffer_starts_empty()
  {
    vix::io::Buffer buffer;
    assert(buffer.empty());
    assert(buffer.size() == 0);
  }

  void test_buffer_append_text()
  {
    vix::io::Buffer buffer;
    buffer.append("Hello");
    buffer.append(" ");
    buffer.append("Vix");

    assert(!buffer.empty());
    assert(buffer.to_string() == "Hello Vix");
    assert(buffer.size() == 9);
  }

  void test_buffer_append_bytes()
  {
    vix::io::Buffer buffer;
    std::vector<std::uint8_t> bytes{65, 66, 67};

    buffer.append(bytes);

    assert(buffer.size() == 3);
    assert(buffer.to_string() == "ABC");
  }

  void test_buffer_clear()
  {
    vix::io::Buffer buffer("hello");
    assert(!buffer.empty());

    buffer.clear();

    assert(buffer.empty());
    assert(buffer.size() == 0);
  }
}

int main()
{
  test_buffer_starts_empty();
  test_buffer_append_text();
  test_buffer_append_bytes();
  test_buffer_clear();
  return 0;
}
