#include <vix/console.hpp>

int main()
{
  vix::console.set_async(false);
  vix::console.set_level(vix::Console::Level::Trace);
  vix::console.set_format(vix::log::LogFormat::KV);

  vix::console.trace("trace details", 1);
  vix::console.debug("debug details", 2);
  vix::console.log("plain log", 3);
  vix::console.info("info message", 4);
  vix::console.warn("warning message", 5);
  vix::console.error("error message", 6);
  vix::console.critical("critical message", 7);

  vix::console.set_level(vix::Console::Level::Warn);
  vix::console.info("this info record is filtered out");
  vix::console.warn("warn still passes");

  vix::console.set_level(vix::Console::Level::Off);
  vix::console.critical("nothing is emitted when level is Off");

  return 0;
}
