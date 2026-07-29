#include <vix/console.hpp>

int main()
{
  vix::console.set_async(false);
  vix::console.set_level(vix::Console::Level::Info);
  vix::console.set_format(vix::log::LogFormat::JSON);

  vix::log::LogContext context;
  context.request_id = "req-123";
  context.module = "http";
  context.fields["tenant"] = "softadastra";
  context.fields["user_id"] = "42";

  vix::console.set_context(context);
  vix::console.log("request started");
  vix::console.event(
      "request.completed",
      vix::Console::field("status", 200),
      vix::Console::field("duration_ms", 12.5));

  const vix::log::LogContext copy = vix::console.context();
  vix::console.log("active request id is", copy.request_id);

  vix::console.clear_context();
  vix::console.log("context fields are no longer attached");

  return 0;
}
