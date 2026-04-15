/**
 *
 *  @file Buffer.hpp
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
#ifndef VIX_IO_BUFFER_HPP
#define VIX_IO_BUFFER_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace vix::io
{

  /**
   * @class Buffer
   * @brief Simple in-memory byte buffer for I/O operations.
   *
   * This is a lightweight public buffer abstraction designed for
   * developer-friendly APIs.
   */
  class Buffer
  {
  public:
    using value_type = std::uint8_t;
    using storage_type = std::vector<value_type>;

    /**
     * @brief Construct an empty buffer.
     */
    Buffer() = default;

    /**
     * @brief Construct a buffer from raw bytes.
     */
    explicit Buffer(storage_type data)
        : data_(std::move(data))
    {
    }

    /**
     * @brief Construct a buffer from a text string.
     */
    explicit Buffer(std::string_view text)
        : data_(text.begin(), text.end())
    {
    }

    /**
     * @brief Return true if the buffer is empty.
     */
    [[nodiscard]] bool empty() const noexcept
    {
      return data_.empty();
    }

    /**
     * @brief Return the number of bytes in the buffer.
     */
    [[nodiscard]] std::size_t size() const noexcept
    {
      return data_.size();
    }

    /**
     * @brief Remove all bytes from the buffer.
     */
    void clear() noexcept
    {
      data_.clear();
    }

    /**
     * @brief Append raw bytes to the buffer.
     */
    void append(const value_type *data, std::size_t size)
    {
      if (data == nullptr || size == 0)
      {
        return;
      }

      data_.insert(data_.end(), data, data + size);
    }

    /**
     * @brief Append a byte vector to the buffer.
     */
    void append(const storage_type &data)
    {
      if (data.empty())
      {
        return;
      }

      data_.insert(data_.end(), data.begin(), data.end());
    }

    /**
     * @brief Append text bytes to the buffer.
     */
    void append(std::string_view text)
    {
      if (text.empty())
      {
        return;
      }

      data_.insert(data_.end(), text.begin(), text.end());
    }

    /**
     * @brief Access the underlying bytes.
     */
    [[nodiscard]] const storage_type &bytes() const noexcept
    {
      return data_;
    }

    /**
     * @brief Access the underlying bytes.
     */
    [[nodiscard]] storage_type &bytes() noexcept
    {
      return data_;
    }

    /**
     * @brief Return a pointer to the underlying bytes.
     */
    [[nodiscard]] const value_type *data() const noexcept
    {
      return data_.data();
    }

    /**
     * @brief Return a pointer to the underlying bytes.
     */
    [[nodiscard]] value_type *data() noexcept
    {
      return data_.data();
    }

    /**
     * @brief Convert the buffer to a text string.
     */
    [[nodiscard]] std::string to_string() const
    {
      return std::string(data_.begin(), data_.end());
    }

  private:
    storage_type data_;
  };

} // namespace vix::io

#endif // VIX_IO_BUFFER_HPP
