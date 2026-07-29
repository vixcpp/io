#include <vix/console.hpp>

#include <cstdlib>

namespace
{
  void set_env(const char *name, const char *value)
  {
#ifdef _WIN32
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
  }
} // namespace

int main()
{
  set_env("VIX_CONSOLE_LEVEL", "debug");
  set_env("VIX_CONSOLE_RATE_LIMIT", "5");
  set_env("VIX_CONSOLE_MAX_DEPTH", "3");
  set_env("VIX_CONSOLE_MAX_ITEMS", "4");
  set_env("VIX_CONSOLE_MAX_STRING_LENGTH", "64");
  set_env("VIX_CONSOLE_MAX_RECORD_SIZE", "512");
  set_env("VIX_LOG_FORMAT", "json");

  vix::Console local_console;
  local_console.set_async(false);
  local_console.set_format(local_console.format());

  local_console.debug("constructed after environment setup");
  local_console.log(
      "config",
      "level", static_cast<int>(local_console.level()),
      "rate", local_console.rate_limit().max_per_second,
      "max_items", local_console.limits().max_items);

  return 0;
}
