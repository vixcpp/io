/**
 *
 *  @file ReadLine.hpp
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
#ifndef VIX_IO_READLINE_HPP
#define VIX_IO_READLINE_HPP

#include <vix/io/Input.hpp>
#include <vix/io/IoOptions.hpp>
#include <vix/io/IoResult.hpp>

namespace vix::io
{

  /**
   * @brief Read a single line from an input stream.
   *
   * By default, the returned string does not include the trailing newline.
   */
  [[nodiscard]] IoStringResult read_line(Input &input,
                                         const IoOptions &options = {});

} // namespace vix::io

#endif // VIX_IO_READLINE_HPP
