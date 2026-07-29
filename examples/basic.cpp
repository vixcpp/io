#include <vix/io/Io.hpp>

int main()
{
  auto out = vix::io::stdout_stream();

  (void)vix::io::write_line(out, "Hello Vix IO");

  return 0;
}
