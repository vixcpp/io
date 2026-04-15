/**
 *
 *  @file IoResult.hpp
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
#ifndef VIX_IO_IORESULT_HPP
#define VIX_IO_IORESULT_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <vix/error/Result.hpp>

namespace vix::io
{

  /**
   * @brief Standard binary buffer type used by I/O APIs.
   */
  using Bytes = std::vector<std::uint8_t>;

  /**
   * @brief Standard result type for I/O operations returning success only.
   */
  using IoBoolResult = vix::error::Result<bool>;

  /**
   * @brief Standard result type for I/O operations returning text.
   */
  using IoStringResult = vix::error::Result<std::string>;

  /**
   * @brief Standard result type for I/O operations returning bytes.
   */
  using IoBytesResult = vix::error::Result<Bytes>;

  /**
   * @brief Standard result type for I/O operations returning a size.
   *
   * Typical use cases:
   * - number of bytes read
   * - number of bytes written
   */
  using IoSizeResult = vix::error::Result<std::uintmax_t>;

} // namespace vix::io

#endif // VIX_IO_IORESULT_HPP
