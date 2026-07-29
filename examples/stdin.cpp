#include <vix/io/Io.hpp>

int main()
{
  auto in = vix::io::stdin_stream();
  auto out = vix::io::stdout_stream();

  auto line = vix::io::read_line(in);

  if (line)
  {
    (void)vix::io::write_line(out, "You typed:");
    (void)vix::io::write_line(out, line.value());
  }
  else
  {
    auto err = vix::io::stderr_stream();
    (void)vix::io::write_line(err, "Failed to read input");
  }

  return 0;
}
