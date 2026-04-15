/**
 *
 *  @file Stdout.hpp
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
#ifndef VIX_IO_STDOUT_HPP
#define VIX_IO_STDOUT_HPP

#include <vix/io/Output.hpp>

namespace vix::io
{

  /**
   * @brief Return a wrapper around the process standard output stream.
   */
  [[nodiscard]] Output stdout_stream() noexcept;

} // namespace vix::io

#endif // VIX_IO_STDOUT_HPP
