/**
 *
 *  @file Copy.cpp
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

#include <algorithm>

#include <vix/io/Copy.hpp>

namespace vix::io
{

  IoSizeResult copy(Input &input,
                    Output &output,
                    const IoOptions &options)
  {
    const std::size_t chunk_size =
        std::max<std::size_t>(options.chunk_size, 1);

    std::uintmax_t total_written = 0;

    while (true)
    {
      auto chunk = input.read(chunk_size);
      if (!chunk)
      {
        return chunk.error();
      }

      if (chunk.value().empty())
      {
        break;
      }

      auto written = output.write(chunk.value());
      if (!written)
      {
        return written.error();
      }

      total_written += written.value();

      if (chunk.value().size() < chunk_size)
      {
        break;
      }
    }

    if (options.auto_flush)
    {
      auto flushed = output.flush();
      if (!flushed)
      {
        return flushed.error();
      }
    }

    return total_written;
  }

} // namespace vix::io
