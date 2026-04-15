/**
 *
 *  @file Copy.hpp
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
#ifndef VIX_IO_COPY_HPP
#define VIX_IO_COPY_HPP

#include <vix/io/Input.hpp>
#include <vix/io/IoOptions.hpp>
#include <vix/io/IoResult.hpp>
#include <vix/io/Output.hpp>

namespace vix::io
{

  /**
   * @brief Copy all remaining bytes from an input stream to an output stream.
   *
   * Returns the total number of bytes written.
   */
  [[nodiscard]] IoSizeResult copy(Input &input,
                                  Output &output,
                                  const IoOptions &options = {});

} // namespace vix::io

#endif // VIX_IO_COPY_HPP
