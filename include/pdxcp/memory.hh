/**
 * @file memory.hh
 * @author Derek Huang
 * @brief C++ header for memory management helpers
 * @copyright MIT License
 */

#ifndef PDXCP_MEMORY_HH_
#define PDXCP_MEMORY_HH_

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace pdxcp {

/**
 * Deleter for a `unique_ptr` that uses `std::free` to free memory.
 *
 * This works for all `unique_ptr` element types since void pointer is used.
 */
struct malloc_deleter {
  /**
   * Free the allocated memory using `std::free`.
   *
   * @param ptr Owned pointer to call `std::free` on
   */
  void operator()(void* ptr) const noexcept
  {
    std::free(ptr);
  }
};

/**
 * Class wrapping a `FILE*` with unique ownership.
 */
class unique_file_stream {
public:
  /**
   * Ctor.
   *
   * @param stream `FILE*` to own, can be `NULL`
   */
  explicit unique_file_stream(FILE* stream) noexcept : stream_{stream} {}

  /**
   * Deleted copy ctor.
   */
  unique_file_stream(const unique_file_stream& other) = delete;

  /**
   * Move ctor.
   *
   * @param other `unique_file_stream` to move from
   */
  unique_file_stream(unique_file_stream&& other) noexcept : stream_{other.stream_}
  {
    other.stream_ = nullptr;
  }

  /**
   * Move assignment operator.
   *
   * @param other `unique_file_stream` to move assign from
   */
  auto& operator=(unique_file_stream&& other) noexcept
  {
    close_stream();
    stream_ = other.stream_;
    other.stream_ = nullptr;
    return *this;
  }

  /**
   * Dtor.
   */
  ~unique_file_stream()
  {
    close_stream();
  }

  /**
   * Return the underlying `FILE*` stream.
   */
  auto stream() const noexcept { return stream_; }

  /**
   * Implicitly convert to `FILE*`.
   */
  operator FILE*() const noexcept { return stream_; }

private:
  FILE* stream_;

  /**
   * Close the underlying `FILE*` if it is not `nullptr`.
   *
   * @note Errors are ignored but the `FILE*` will never be used again.
   */
  void close_stream() noexcept
  {
    std::fclose(stream_);  // errors ignored
  }
};

}  // namespace pdxcp

#endif  // PDXCP_MEMORY_HH_
