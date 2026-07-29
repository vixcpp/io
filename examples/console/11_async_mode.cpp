#include <vix/console.hpp>

int main()
{
  vix::console.set_level(vix::Console::Level::Trace);
  vix::console.set_format(vix::log::LogFormat::JSON);

  vix::console.set_async(true);
  for (int i = 0; i < 5; ++i)
  {
    vix::console.log("async record", i);
  }

  vix::console.set_async(false);
  vix::console.log("back to synchronous logging");

  return 0;
}
