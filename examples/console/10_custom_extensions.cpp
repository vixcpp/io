#include <vix/console.hpp>

#include <ostream>
#include <string>

namespace demo
{
  struct Color
  {
    int r;
    int g;
    int b;
  };

  struct Point
  {
    int x;
    int y;
  };

  inline void vix_format(std::ostream &os, const Point &point)
  {
    os << "Point(" << point.x << ", " << point.y << ')';
  }

  struct Result
  {
    std::string label;
    double value;
  };

  struct User
  {
    std::string name;
    int age;
  };
} // namespace demo

template <>
struct vix::formatter<demo::Color>
{
  static void format(std::ostream &os, const demo::Color &color)
  {
    os << "rgb(" << color.r << ", " << color.g << ", " << color.b << ')';
  }
};

template <>
struct vix::inspector<demo::Result>
{
  static void inspect(vix::inspect_context &ctx, const demo::Result &result)
  {
    ctx.os << result.label << '=' << result.value;
  }
};

template <>
struct vix::field_map<demo::User>
{
  static constexpr auto fields()
  {
    return vix::fields(
        vix::field("name", &demo::User::name),
        vix::field("age", &demo::User::age));
  }
};

int main()
{
  vix::console.set_async(false);
  vix::console.set_level(vix::Console::Level::Trace);
  vix::console.set_format(vix::log::LogFormat::JSON);

  vix::console.log("formatter specialization", demo::Color{20, 80, 200});
  vix::console.log("ADL vix_format hook", demo::Point{3, 7});
  vix::console.dir(demo::Result{"score", 98.5});
  vix::console.dir(demo::User{"Ada", 37});

  return 0;
}
