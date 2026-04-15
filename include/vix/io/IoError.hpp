/**
 *
 *  @file IoError.hpp
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
#ifndef VIX_IO_IOERROR_HPP
#define VIX_IO_IOERROR_HPP

#include <string>

#include <vix/error/Error.hpp>
#include <vix/error/ErrorCategory.hpp>
#include <vix/error/ErrorCode.hpp>

namespace vix::io
{

  /**
   * @enum IoErrorCode
   * @brief I/O-specific semantic error codes.
   *
   * These codes describe common failures while reading from or writing to
   * streams, buffers, and standard input/output channels.
   */
  enum class IoErrorCode
  {
    None = 0,
    InvalidInput,
    InvalidOutput,
    InvalidOperation,
    ReadFailed,
    WriteFailed,
    FlushFailed,
    EndOfStream,
    Closed,
    Timeout,
    Unknown
  };

  /**
   * @brief Return the default I/O error category.
   */
  [[nodiscard]] inline constexpr vix::error::ErrorCategory io_error_category() noexcept
  {
    return vix::error::ErrorCategory("io");
  }

  /**
   * @brief Convert an IoErrorCode to a generic Vix ErrorCode.
   */
  [[nodiscard]] inline constexpr vix::error::ErrorCode to_error_code(IoErrorCode code) noexcept
  {
    using vix::error::ErrorCode;

    switch (code)
    {
    case IoErrorCode::None:
      return ErrorCode::Ok;

    case IoErrorCode::InvalidInput:
    case IoErrorCode::InvalidOutput:
    case IoErrorCode::InvalidOperation:
      return ErrorCode::InvalidArgument;

    case IoErrorCode::ReadFailed:
    case IoErrorCode::WriteFailed:
    case IoErrorCode::FlushFailed:
      return ErrorCode::IoError;

    case IoErrorCode::EndOfStream:
      return ErrorCode::NotFound;

    case IoErrorCode::Closed:
      return ErrorCode::InvalidState;

    case IoErrorCode::Timeout:
      return ErrorCode::Timeout;

    case IoErrorCode::Unknown:
      return ErrorCode::Unknown;
    }

    return ErrorCode::Unknown;
  }

  /**
   * @brief Convert an IoErrorCode to a stable human-readable name.
   */
  [[nodiscard]] inline const char *to_string(IoErrorCode code) noexcept
  {
    switch (code)
    {
    case IoErrorCode::None:
      return "none";
    case IoErrorCode::InvalidInput:
      return "invalid_input";
    case IoErrorCode::InvalidOutput:
      return "invalid_output";
    case IoErrorCode::InvalidOperation:
      return "invalid_operation";
    case IoErrorCode::ReadFailed:
      return "read_failed";
    case IoErrorCode::WriteFailed:
      return "write_failed";
    case IoErrorCode::FlushFailed:
      return "flush_failed";
    case IoErrorCode::EndOfStream:
      return "end_of_stream";
    case IoErrorCode::Closed:
      return "closed";
    case IoErrorCode::Timeout:
      return "timeout";
    case IoErrorCode::Unknown:
      return "unknown";
    }

    return "unknown";
  }

  /**
   * @brief Build a structured Vix error from an IoErrorCode.
   *
   * @param code I/O-specific error code.
   * @param message Human-readable message.
   */
  [[nodiscard]] inline vix::error::Error make_io_error(IoErrorCode code, std::string message)
  {
    return vix::error::Error(
        to_error_code(code),
        io_error_category(),
        std::move(message));
  }

} // namespace vix::io

#endif // VIX_IO_IOERROR_HPP
