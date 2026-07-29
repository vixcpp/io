#include <vix/console.hpp>

int main()
{
  vix::console.set_async(false);
  vix::console.set_level(vix::Console::Level::Trace);

  vix::console.set_format(vix::log::LogFormat::KV);
  vix::console.log("kv format", "simple human-readable output");

  vix::console.set_format(vix::log::LogFormat::JSON);
  vix::console.event(
      "json.single_line",
      vix::Console::field("status", 200),
      vix::Console::field("success", true),
      vix::Console::field("duration_ms", 4.25));

  vix::console.set_format(vix::log::LogFormat::JSON_PRETTY);
  vix::console.event(
      "json.pretty",
      vix::Console::field("method", "GET"),
      vix::Console::field("path", "/health"),
      vix::Console::field("status", 200));

  return 0;
}
