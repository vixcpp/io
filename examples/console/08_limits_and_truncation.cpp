#include <vix/console.hpp>

#include <map>
#include <string>
#include <vector>

int main()
{
  vix::console.set_async(false);
  vix::console.set_level(vix::Console::Level::Trace);
  vix::console.set_format(vix::log::LogFormat::JSON);

  vix::console.set_limits({
      2,   // max_depth
      3,   // max_items
      32,  // max_string_length
      160  // max_record_size
  });

  vix::console.log("long string", std::string(120, 'x'));

  std::vector<int> many_items{1, 2, 3, 4, 5, 6, 7, 8};
  vix::console.log("limited items", many_items);

  std::map<std::string, std::vector<std::vector<int>>> deep{
      {"levels", {{1, 2}, {3, 4}}}};
  vix::console.log("limited depth", deep);

  vix::console.log("bounded record", std::string(500, 'r'));

  const auto limits = vix::console.limits();
  vix::console.log(
      "current limits",
      "depth", limits.max_depth,
      "items", limits.max_items,
      "string", limits.max_string_length,
      "record", limits.max_record_size);

  return 0;
}
