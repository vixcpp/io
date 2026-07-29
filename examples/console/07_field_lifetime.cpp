#include <vix/console.hpp>

#include <string>
#include <type_traits>

int main()
{
  vix::console.set_async(false);
  vix::console.set_level(vix::Console::Level::Trace);
  vix::console.set_format(vix::log::LogFormat::JSON);

  int status = 200;
  auto status_field = vix::Console::field("status", status);
  static_assert(
      std::is_same_v<decltype(status_field), vix::Console::FieldRef<int>>);

  status = 201;
  vix::console.event("lvalue.field.ref", status_field);

  auto owned_name = vix::Console::field("name", std::string{"temporary"});
  static_assert(
      std::is_same_v<decltype(owned_name),
                     vix::Console::FieldValue<std::string>>);

  vix::console.event("rvalue.field.value", owned_name);

  return 0;
}
