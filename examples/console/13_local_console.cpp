#include <vix/console.hpp>

int main()
{
  vix::Console console;

  console.set_async(false);
  console.set_level(vix::Console::Level::Trace);
  console.set_format(vix::log::LogFormat::JSON);
  console.set_rate_limit({false, 0});

  console.log("local console instance");

  vix::Console::Config config;
  config.level = vix::Console::Level::Warn;
  config.format = vix::log::LogFormat::KV;
  config.async = false;
  config.limits = {4, 8, 128, 2048};
  config.rate_limit = {false, 0};
  console.configure(config);

  if (console.enabled(vix::Console::Level::Info))
  {
    console.info("this is not emitted with Warn level");
  }

  console.warn("configured local console");
  console.clear_context();

  return 0;
}
