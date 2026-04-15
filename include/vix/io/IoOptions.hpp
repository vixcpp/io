/**
 *
 *  @file IoOptions.hpp
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
#ifndef VIX_IO_IOOPTIONS_HPP
#define VIX_IO_IOOPTIONS_HPP

#include <cstddef>

namespace vix::io
{

  /**
   * @enum NewlineMode
   * @brief Newline behavior used by line-oriented APIs.
   */
  enum class NewlineMode
  {
    LF,
    CRLF,
    Native
  };

  /**
   * @struct IoOptions
   * @brief Common options reused by I/O operations.
   *
   * The public API stays intentionally simple while allowing the module
   * to evolve later.
   */
  struct IoOptions
  {
    /**
     * @brief Preferred chunk size when reading or copying.
     */
    std::size_t chunk_size{4096};

    /**
     * @brief Whether trailing newline characters should be preserved
     * when using line-oriented reads.
     */
    bool keep_newline{false};

    /**
     * @brief Whether writes should flush automatically.
     */
    bool auto_flush{false};

    /**
     * @brief Preferred newline mode for line-oriented writes.
     */
    NewlineMode newline_mode{NewlineMode::LF};
  };

} // namespace vix::io

#endif // VIX_IO_IOOPTIONS_HPP
