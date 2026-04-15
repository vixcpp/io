/**
 *
 *  @file Output.hpp
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
#ifndef VIX_IO_OUTPUT_HPP
#define VIX_IO_OUTPUT_HPP

#include <ostream>
#include <string_view>

#include <vix/io/Buffer.hpp>
#include <vix/io/IoResult.hpp>

namespace vix::io
{

  /**
   * @class Output
   * @brief Simple wrapper around a writable output stream.
   *
   * This class provides a lightweight, developer-friendly abstraction for
   * stream-based output without exposing unnecessary complexity.
   */
  class Output
  {
  public:
    /**
     * @brief Construct an Output wrapper from a standard output stream.
     */
    explicit Output(std::ostream &stream) noexcept;

    /**
     * @brief Write raw bytes to the stream.
     *
     * Returns the number of bytes written.
     */
    [[nodiscard]] IoSizeResult write(const Bytes &data);

    /**
     * @brief Write text to the stream.
     *
     * Returns the number of bytes written.
     */
    [[nodiscard]] IoSizeResult write(std::string_view text);

    /**
     * @brief Flush the stream.
     */
    [[nodiscard]] IoBoolResult flush();

    /**
     * @brief Return true if the stream is still in a writable state.
     */
    [[nodiscard]] bool good() const noexcept;

    /**
     * @brief Access the underlying standard stream.
     */
    [[nodiscard]] std::ostream &stream() noexcept;

  private:
    std::ostream *stream_{nullptr};
  };

} // namespace vix::io

#endif // VIX_IO_OUTPUT_HPP
