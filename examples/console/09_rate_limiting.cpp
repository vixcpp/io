#include <vix/console.hpp>

int main()
{
  vix::console.set_async(false);
  vix::console.set_level(vix::Console::Level::Trace);
  vix::console.set_format(vix::log::LogFormat::KV);

  vix::console.set_rate_limit({true, 3});

  for (int i = 0; i < 10; ++i)
  {
    vix::console.debug("debug record", i);
  }

  vix::console.warn("warn is never rate limited");
  vix::console.error("error is never rate limited");
  vix::console.critical("critical is never rate limited");

  vix::console.set_rate_limit({false, 0});
  vix::console.log("rate limiting disabled again");

  return 0;
}
