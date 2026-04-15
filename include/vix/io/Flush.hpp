/**
 *
 *  @file Flush.hpp
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
#ifndef VIX_IO_FLUSH_HPP
#define VIX_IO_FLUSH_HPP

#include <vix/io/IoResult.hpp>
#include <vix/io/Output.hpp>

namespace vix::io
{

  /**
   * @brief Flush an output stream.
   */
  [[nodiscard]] IoBoolResult flush(Output &output);

} // namespace vix::io

#endif // VIX_IO_FLUSH_HPP
