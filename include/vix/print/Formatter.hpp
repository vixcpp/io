/** @file Formatter.hpp Shared declaration of the vix::print extension point. */
#ifndef VIX_PRINT_FORMATTER_HPP
#define VIX_PRINT_FORMATTER_HPP

namespace vix
{
  /** Specialize formatter<T> to render T through the legacy ostream hook. */
  template <typename T, typename = void>
  struct formatter
  {
  };
}

#endif
