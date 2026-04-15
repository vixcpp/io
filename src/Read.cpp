/**
 *
 *  @file Read.cpp
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

#include <vix/io/Read.hpp>

namespace vix::io
{

  IoBytesResult read(Input &input, std::size_t max_bytes)
  {
    return input.read(max_bytes);
  }

} // namespace vix::io
