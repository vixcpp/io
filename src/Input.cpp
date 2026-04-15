/**
 *
 *  @file Input.cpp
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

#include <string>
#include <vector>

#include <vix/io/Input.hpp>
#include <vix/io/IoError.hpp>

namespace vix::io
{

  Input::Input(std::istream &stream) noexcept
      : stream_(&stream)
  {
  }

  IoBytesResult Input::read(std::size_t max_bytes)
  {
    if (stream_ == nullptr)
    {
      return make_io_error(IoErrorCode::InvalidInput, "input stream is null");
    }

    if (max_bytes == 0)
    {
      return Bytes{};
    }

    std::vector<char> temp(max_bytes, '\0');
    stream_->read(temp.data(), static_cast<std::streamsize>(temp.size()));

    const std::streamsize count = stream_->gcount();
    if (count < 0)
    {
      return make_io_error(IoErrorCode::ReadFailed, "failed to read from input stream");
    }

    if (stream_->bad())
    {
      return make_io_error(IoErrorCode::ReadFailed, "input stream is in a bad state");
    }

    Bytes out;
    out.reserve(static_cast<std::size_t>(count));

    for (std::streamsize i = 0; i < count; ++i)
    {
      out.push_back(static_cast<std::uint8_t>(temp[static_cast<std::size_t>(i)]));
    }

    return out;
  }

  IoStringResult Input::read_line()
  {
    if (stream_ == nullptr)
    {
      return make_io_error(IoErrorCode::InvalidInput, "input stream is null");
    }

    std::string line;
    if (!std::getline(*stream_, line))
    {
      if (stream_->eof())
      {
        return std::string{};
      }

      return make_io_error(IoErrorCode::ReadFailed, "failed to read line from input stream");
    }

    return line;
  }

  bool Input::good() const noexcept
  {
    return stream_ != nullptr && stream_->good();
  }

  bool Input::eof() const noexcept
  {
    return stream_ == nullptr || stream_->eof();
  }

  std::istream &Input::stream() noexcept
  {
    return *stream_;
  }

} // namespace vix::io
