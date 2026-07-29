#include <vix/console.hpp>

#include <map>
#include <string>
#include <vector>

struct Route
{
  std::string method;
  std::string path;
  std::vector<int> status_codes;
};

template <>
struct vix::field_map<Route>
{
  static constexpr auto fields()
  {
    return vix::fields(
        vix::field("method", &Route::method),
        vix::field("path", &Route::path),
        vix::field("status_codes", &Route::status_codes));
  }
};

int main()
{
  vix::console.set_async(false);
  vix::console.set_level(vix::Console::Level::Trace);
  vix::console.set_format(vix::log::LogFormat::JSON);

  std::map<std::string, std::vector<int>> index{
      {"users", {200, 404}},
      {"orders", {200, 409, 500}}};

  vix::console.dir(index);

  vix::inspect_options compact = vix::default_options();
  compact.compact = true;
  compact.show_type = false;
  compact.max_depth = 2;
  compact.max_items = 2;
  vix::console.dir(index, compact);

  Route route{"GET", "/users/{id}", {200, 404}};
  vix::console.dir(route);

  return 0;
}
