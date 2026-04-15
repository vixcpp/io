/**
 *
 *  @file Write.hpp
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
#ifndef VIX_IO_WRITE_HPP
#define VIX_IO_WRITE_HPP

#include <string_view>

#include <vix/io/IoResult.hpp>
#include <vix/io/Output.hpp>

namespace vix::io
{

  /**
   * @brief Write raw bytes to an output stream.
   *
   * Returns the number of bytes written.
   */
  [[nodiscard]] IoSizeResult write(Output &output, const Bytes &data);

  /**
   * @brief Write text to an output stream.
   *
   * Returns the number of bytes written.
   */
  [[nodiscard]] IoSizeResult write(Output &output, std::string_view text);

} // namespace vix::io

#endif // VIX_IO_WRITE_HPP
