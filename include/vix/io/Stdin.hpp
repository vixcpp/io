/**
 *
 *  @file Stdin.hpp
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
#ifndef VIX_IO_STDIN_HPP
#define VIX_IO_STDIN_HPP

#include <vix/io/Input.hpp>

namespace vix::io
{

  /**
   * @brief Return a wrapper around the process standard input stream.
   */
  [[nodiscard]] Input stdin_stream() noexcept;

} // namespace vix::io

#endif // VIX_IO_STDIN_HPP
