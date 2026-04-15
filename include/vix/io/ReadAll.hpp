/**
 *
 *  @file ReadAll.hpp
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
#ifndef VIX_IO_READALL_HPP
#define VIX_IO_READALL_HPP

#include <vix/io/Input.hpp>
#include <vix/io/IoOptions.hpp>
#include <vix/io/IoResult.hpp>

namespace vix::io
{

  /**
   * @brief Read all remaining bytes from an input stream.
   */
  [[nodiscard]] IoBytesResult read_all(Input &input,
                                       const IoOptions &options = {});

} // namespace vix::io

#endif // VIX_IO_READALL_HPP
