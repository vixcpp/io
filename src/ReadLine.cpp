/**
 *
 *  @file ReadLine.cpp
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

#include <vix/io/ReadLine.hpp>

namespace vix::io
{

  IoStringResult read_line(Input &input, const IoOptions &options)
  {
    auto line = input.read_line();
    if (!line)
    {
      return line.error();
    }

    if (options.keep_newline)
    {
      line.value().push_back('\n');
    }

    return line;
  }

} // namespace vix::io
