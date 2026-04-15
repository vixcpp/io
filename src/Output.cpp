/**
 *
 *  @file Output.cpp
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

#include <vix/io/IoError.hpp>
#include <vix/io/Output.hpp>

namespace vix::io
{

  Output::Output(std::ostream &stream) noexcept
      : stream_(&stream)
  {
  }

  IoSizeResult Output::write(const Bytes &data)
  {
    if (stream_ == nullptr)
    {
      return make_io_error(IoErrorCode::InvalidOutput, "output stream is null");
    }

    if (data.empty())
    {
      return static_cast<std::uintmax_t>(0);
    }

    stream_->write(
        reinterpret_cast<const char *>(data.data()),
        static_cast<std::streamsize>(data.size()));

    if (!(*stream_))
    {
      return make_io_error(IoErrorCode::WriteFailed, "failed to write bytes to output stream");
    }

    return static_cast<std::uintmax_t>(data.size());
  }

  IoSizeResult Output::write(std::string_view text)
  {
    if (stream_ == nullptr)
    {
      return make_io_error(IoErrorCode::InvalidOutput, "output stream is null");
    }

    if (text.empty())
    {
      return static_cast<std::uintmax_t>(0);
    }

    stream_->write(text.data(), static_cast<std::streamsize>(text.size()));

    if (!(*stream_))
    {
      return make_io_error(IoErrorCode::WriteFailed, "failed to write text to output stream");
    }

    return static_cast<std::uintmax_t>(text.size());
  }

  IoBoolResult Output::flush()
  {
    if (stream_ == nullptr)
    {
      return make_io_error(IoErrorCode::InvalidOutput, "output stream is null");
    }

    stream_->flush();

    if (!(*stream_))
    {
      return make_io_error(IoErrorCode::FlushFailed, "failed to flush output stream");
    }

    return true;
  }

  bool Output::good() const noexcept
  {
    return stream_ != nullptr && stream_->good();
  }

  std::ostream &Output::stream() noexcept
  {
    return *stream_;
  }

} // namespace vix::io
