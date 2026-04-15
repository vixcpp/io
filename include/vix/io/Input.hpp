/**
 *
 *  @file Input.hpp
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
#ifndef VIX_IO_INPUT_HPP
#define VIX_IO_INPUT_HPP

#include <cstddef>
#include <istream>

#include <vix/io/Buffer.hpp>
#include <vix/io/IoResult.hpp>

namespace vix::io
{

  /**
   * @class Input
   * @brief Simple wrapper around a readable input stream.
   *
   * This class provides a lightweight, developer-friendly abstraction for
   * stream-based input without exposing unnecessary complexity.
   */
  class Input
  {
  public:
    /**
     * @brief Construct an Input wrapper from a standard input stream.
     */
    explicit Input(std::istream &stream) noexcept;

    /**
     * @brief Read up to max_bytes from the stream.
     *
     * Returns the bytes actually read. An empty buffer may mean end of stream.
     */
    [[nodiscard]] IoBytesResult read(std::size_t max_bytes);

    /**
     * @brief Read a full line from the stream.
     *
     * The returned string does not include the trailing newline.
     */
    [[nodiscard]] IoStringResult read_line();

    /**
     * @brief Return true if the stream is still in a readable state.
     */
    [[nodiscard]] bool good() const noexcept;

    /**
     * @brief Return true if the stream has reached end-of-stream.
     */
    [[nodiscard]] bool eof() const noexcept;

    /**
     * @brief Access the underlying standard stream.
     */
    [[nodiscard]] std::istream &stream() noexcept;

  private:
    std::istream *stream_{nullptr};
  };

} // namespace vix::io

#endif // VIX_IO_INPUT_HPP
