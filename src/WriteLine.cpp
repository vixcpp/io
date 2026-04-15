/**
 *
 *  @file WriteLine.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */

#include <string>

#include <vix/io/Flush.hpp>
#include <vix/io/IoError.hpp>
#include <vix/io/WriteLine.hpp>

namespace vix::io
{

  namespace
  {
    [[nodiscard]] const char *newline_text(NewlineMode mode) noexcept
    {
      switch (mode)
      {
      case NewlineMode::LF:
        return "\n";
      case NewlineMode::CRLF:
        return "\r\n";
      case NewlineMode::Native:
#ifdef _WIN32
        return "\r\n";
#else
        return "\n";
#endif
      }

      return "\n";
    }
  } // namespace

  IoSizeResult write_line(Output &output,
                          std::string_view text,
                          const IoOptions &options)
  {
    auto first = output.write(text);
    if (!first)
    {
      return first.error();
    }

    const char *newline = newline_text(options.newline_mode);

    auto second = output.write(std::string_view(newline));
    if (!second)
    {
      return second.error();
    }

    if (options.auto_flush)
    {
      auto flushed = output.flush();
      if (!flushed)
      {
        return flushed.error();
      }
    }

    return first.value() + second.value();
  }

} // namespace vix::io
