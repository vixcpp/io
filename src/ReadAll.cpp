/**
 *
 *  @file ReadAll.cpp
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

#include <vix/io/IoError.hpp>
#include <vix/io/ReadAll.hpp>

namespace vix::io
{

  IoBytesResult read_all(Input &input, const IoOptions &options)
  {
    const std::size_t chunk_size =
        std::max<std::size_t>(options.chunk_size, 1);

    Bytes out;

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

      const auto &bytes = chunk.value();
      out.insert(out.end(), bytes.begin(), bytes.end());

      if (bytes.size() < chunk_size)
      {
        break;
      }
    }

    return out;
  }

} // namespace vix::io
