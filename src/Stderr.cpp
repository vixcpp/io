/**
 *
 *  @file Stderr.cpp
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

#include <iostream>

#include <vix/io/Stderr.hpp>

namespace vix::io
{

  Output stderr_stream() noexcept
  {
    return Output(std::cerr);
  }

} // namespace vix::io
