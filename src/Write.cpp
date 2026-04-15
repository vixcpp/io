/**
 *
 *  @file Write.cpp
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

#include <vix/io/Write.hpp>

namespace vix::io
{

  IoSizeResult write(Output &output, const Bytes &data)
  {
    return output.write(data);
  }

  IoSizeResult write(Output &output, std::string_view text)
  {
    return output.write(text);
  }

} // namespace vix::io
