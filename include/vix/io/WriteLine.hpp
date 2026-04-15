/**
 *
 *  @file WriteLine.hpp
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
#ifndef VIX_IO_WRITELINE_HPP
#define VIX_IO_WRITELINE_HPP

#include <string_view>

#include <vix/io/IoOptions.hpp>
#include <vix/io/IoResult.hpp>
#include <vix/io/Output.hpp>

namespace vix::io
{

  /**
   * @brief Write a line of text to an output stream.
   *
   * A newline is appended according to the provided options.
   * Returns the total number of bytes written.
   */
  [[nodiscard]] IoSizeResult write_line(Output &output,
                                        std::string_view text,
                                        const IoOptions &options = {});

} // namespace vix::io

#endif // VIX_IO_WRITELINE_HPP
