#include <vix/console.hpp>

#include <chrono>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

enum class Role
{
  Admin = 1,
  Guest = 2
};

int main()
{
  vix::console.set_async(false);
  vix::console.set_level(vix::Console::Level::Trace);
  vix::console.set_format(vix::log::LogFormat::JSON);

  const char *missing_name = nullptr;

  vix::console.log(
      "primitive values",
      42,
      42u,
      true,
      false,
      3.14,
      Role::Admin,
      nullptr,
      missing_name);

  std::vector<int> ids{1, 2, 3};
  std::map<std::string, int> counters{{"open", 3}, {"closed", 9}};
  std::unordered_map<std::string, int> tags{{"beta", 1}, {"stable", 2}};
  std::tuple<int, std::string, bool> tuple_value{7, "seven", true};
  std::pair<std::string, int> pair_value{"score", 99};
  std::optional<std::string> maybe_name{"Ada"};
  std::variant<int, std::string> variant_value{std::string{"active"}};
  auto unique_value = std::make_unique<int>(123);
  auto shared_value = std::make_shared<std::string>("shared text");
  std::chrono::milliseconds elapsed{125};
  std::filesystem::path path{"logs/app.log"};

  vix::console.log(
      "complex values",
      ids,
      counters,
      tags,
      tuple_value,
      pair_value,
      maybe_name,
      variant_value,
      unique_value,
      shared_value,
      elapsed,
      path);

  std::map<std::string, std::vector<int>> nested{
      {"first", {1, 2}},
      {"second", {3, 4}}};
  vix::console.log("nested map", nested);

  return 0;
}
