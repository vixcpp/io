/**
 *
 *  @file Stdin.cpp
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

#include <vix/io/Stdin.hpp>

namespace vix::io
{

  Input stdin_stream() noexcept
  {
    return Input(std::cin);
  }

} // namespace vix::io
