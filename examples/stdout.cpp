#include <vix/io/Io.hpp>

int main()
{
  auto out = vix::io::stdout_stream();

  (void)vix::io::write(out, "Hello ");
  (void)vix::io::write_line(out, "world");

  vix::io::Bytes bytes{65, 66, 67};
  (void)vix::io::write_line(out, "\nBytes:");
  (void)vix::io::write(out, bytes);

  return 0;
}
