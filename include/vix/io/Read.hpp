/**
 *
 *  @file Read.hpp
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
#ifndef VIX_IO_READ_HPP
#define VIX_IO_READ_HPP

#include <cstddef>

#include <vix/io/Input.hpp>
#include <vix/io/IoResult.hpp>

namespace vix::io
{

  /**
   * @brief Read up to max_bytes from an input stream.
   *
   * Returns the bytes actually read.
   */
  [[nodiscard]] IoBytesResult read(Input &input, std::size_t max_bytes);

} // namespace vix::io

#endif // VIX_IO_READ_HPP
