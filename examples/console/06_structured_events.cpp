#include <vix/console.hpp>

#include <string>
#include <vector>

int main()
{
  vix::console.set_async(false);
  vix::console.set_level(vix::Console::Level::Trace);
  vix::console.set_format(vix::log::LogFormat::JSON);

  int status = 200;
  std::vector<int> values{1, 2, 3};
  std::string method = "GET";

  vix::console.event(
      "http.request",
      vix::Console::field("method", method),
      vix::Console::field("status", status),
      vix::Console::field("success", true),
      vix::Console::field("duration_ms", 4.5),
      vix::Console::field("values", values));

  vix::console.event_at(
      vix::Console::Level::Warn,
      "cache.miss",
      vix::Console::field("key", "session:42"),
      vix::Console::field("attempt", 1));

  vix::console.event_at(
      vix::Console::Level::Error,
      "payment.failed",
      vix::Console::field("code", "card_declined"),
      vix::Console::field("retryable", false));

  vix::console.event_at(
      vix::Console::Level::Off,
      "this.is.not.emitted",
      vix::Console::field("ignored", true));

  vix::console.event(
      "empty.key",
      vix::Console::field("", 42));

  return 0;
}
