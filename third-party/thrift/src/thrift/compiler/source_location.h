/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstdint>
#include <deque>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace apache::thrift::compiler {

class source_manager;

// A lightweight source location that can be resolved via `source_manager` into
// `resolved_location` that provides the file name, line and column.
class source_location {
 private:
  uint_least32_t source_id_ = 0;
  uint_least32_t offset_ = 0;

  friend class resolved_location;
  friend class source_manager;

  source_location(uint_least32_t source_id, uint_least32_t offset)
      : source_id_(source_id), offset_(offset) {}

 public:
  // Creates a location that doesn't refer to any source.
  source_location() = default;

  friend bool operator==(source_location lhs, source_location rhs) {
    return lhs.source_id_ == rhs.source_id_ && lhs.offset_ == rhs.offset_;
  }
  friend bool operator!=(source_location lhs, source_location rhs) {
    return !(lhs == rhs);
  }

  friend source_location operator+(source_location lhs, int rhs) {
    lhs.offset_ += rhs;
    return lhs;
  }

  // Returns the offset in code units from the beginning of the source.
  uint_least32_t offset() const { return offset_; }
};

struct source_range {
  /**
   * Beginning of this range in source file, inclusive.
   */
  source_location begin;

  /**
   * Beginning of this range in source file, exclusive (i.e., one past the last
   * location included in this range, if any)..
   */
  source_location end;
};

// A resolved (source) location that provides the file name, line and column.
class resolved_location {
 private:
  const char* file_name_;
  unsigned line_;
  unsigned column_;

 public:
  resolved_location(source_location loc, const source_manager& sm);

  // Returns the source file name. It can include directory components and/or be
  // a virtual file name that doesn't have a correspondent entry in the system's
  // directory structure.
  const char* file_name() const { return file_name_; }

  unsigned line() const { return line_; }
  unsigned column() const { return column_; }
};

// A view of a source owned by `source_manager`.
struct source_view final {
  source_location start; // The source start location.
  std::string_view text; // The source text including a terminating '\0'.
};

// DO_BEFORE(aristidis,20251001): Remove alias when all references are updated.
using source = source_view;

// A class that abstracts the reading of files from the file system. The
// backend could read from a real file system, or be an in-memory
// representation itself.
//
// The source_manager will pull sources from the backend as needed, and
// perform indexing on the received contents.
class source_manager_backend {
 public:
  virtual ~source_manager_backend() noexcept = default;
  // Returns the file contents at the provided path, or empty optional if the
  // path is not found.
  virtual std::optional<std::vector<char>> read_file(std::string_view path) = 0;
  // Returns true if and only if the path exists. That is, the read_file(path)
  // call above would return a non-empty optional.
  virtual bool exists(const std::filesystem::path& path) = 0;
};

// A source manager that caches sources in memory, loads files and enables
// resolution of offset-based source locations into file names, lines and
// columns.
class source_manager {
 private:
  std::unique_ptr<source_manager_backend> backend_;

  struct source_info {
    std::string file_name;
    std::vector<char> text;
    std::vector<uint_least32_t> line_offsets;
  };

  // This is a deque to make sure that file_name is not reallocated when
  // sources_ grows.
  std::deque<source_info> sources_;

  /**
   * Maps the file_name of a source file owned by this source_manager (i.e., in
   * `sources_`) to its `source_view`.
   */
  std::map<std::string, source_view, std::less<>> file_source_map_;

  // Maps from filepaths present in the AST to filepaths on disk.
  std::map<std::string, std::string, std::less<>> found_includes_;

  const source_info* get_source(uint_least32_t source_id) const {
    return source_id > 0 && source_id <= sources_.size()
        ? &sources_[source_id - 1]
        : nullptr;
  }

  friend class resolved_location;

  /**
   * Adds the given contents of a source file (with the given name) to this
   * manager (i.e., to `sources_`).
   *
   * @param file_name unique name of the file whose contents are being added.
   *        Behavior is undefined if this name is not unique.
   *
   * @param text file contents. Last element MUST be the NUL character.
   *
   * @return a view into the (newly added) content.
   */
  source_view add_source(std::string_view file_name, std::vector<char> text);

  /**
   * Check if the given `include_path` exists relative to the given
   * `search_path` (in this source_manager's backend filesystem.
   *
   * @return the resolved path, if it exists, or std::nullopt if not.
   */
  std::optional<std::filesystem::path> try_search_path(
      const std::filesystem::path& include_path,
      const std::filesystem::path& search_path);

  bool path_exists_in_backend(const std::filesystem::path& path) const;

 public:
  // Creates a source_manager with the default (filesystem-based) backend.
  source_manager();

  // Creates a source_manager with the user-provided backend implementation.
  // If the backend is null, then only virtual files can be read.
  explicit source_manager(std::unique_ptr<source_manager_backend> backend)
      : backend_(std::move(backend)) {}

  source_manager(source_manager&) noexcept = delete;
  source_manager& operator=(source_manager&) noexcept = delete;
  source_manager(source_manager&&) noexcept = default;
  source_manager& operator=(source_manager&&) noexcept = default;
  ~source_manager() noexcept = default;

  /**
   * Returns a view into the contents of a file with the given name.
   *
   * The file can be a real file (provided by the backend), or a virtual one
   * previously registered with `add_virtual_file()`.
   *
   * The underlying contents are owned by this manager.
   *
   * Returns an empty optional if opening or reading the file fails. Makes use
   * of the result of previous calls to `find_include_file()`.
   */
  std::optional<source_view> get_file(std::string_view file_name);

  std::string get_file_path(std::string_view file_name) const;

  /**
   * Adds a virtual file with the specified name and content.
   *
   * @return a view into the (newly added) content.
   *
   * @throws if a file with the given name already exists.
   */
  source_view add_virtual_file(
      std::string_view file_name, std::string_view src);

  // Returns the start location of a source containing the specified location.
  // It is a member function in case we add clang-like compression of locations.
  source_location get_source_start(source_location loc) const {
    return {loc.source_id_, 0};
  }

  // Returns a pointer to the source text at the specified location or nullptr
  // if the location is invalid.
  const char* get_text(source_location loc) const;

  /**
   * Returns a view of the given source text range.
   *
   * Preconditions:
   *   * Both (inclusive) begin and (exclusive) `end` locations of the given
   *     `range` must be from the same source.
   *   * The range must be well defined (eg., both endpoints must be valid,
   *     `end` must not come before `begin`, etc.)
   *
   * The behavior is undefined if preconditions are not met (if assertions are
   * enabled, the process should abort).
   */
  std::string_view get_text_range(const source_range& range) const;

  /**
   * Attempts to locate an included file.
   *
   * @param include_file_path The path to the included file, as specified in
   *        Thrift IDL source (i.e., the path that follows the `include`
   *        keyword).
   * @param search_paths The ordered list of (directory) paths explicitly
   *        provided by the caller, relative to which to attempt resolving the
   *        given `include_file_path`.
   * @param program_path If present, the path to the file of the program
   *        currently being compiled, which is trying to `include
   *        "include_file_path"`. The include path will be resolved against the
   *        directory of the `program_path` before trying `search_paths`.
   *
   * If the file corresponding to `include_file_path` is successfully found, its
   * path is returned. The contents of that file are added to this
   * `source_manager` instance, and are available through `get_file()` (using
   * the returned path).
   */
  using path_or_error = std::variant<std::string, std::string>;
  path_or_error find_include_file(
      std::string_view include_file_path,
      const std::vector<std::string>& search_paths,
      std::optional<std::string_view> program_path);

  // Queries for a file previously found by find_include_file.
  std::optional<std::string> found_include_file(
      std::string_view filename) const;
};

/**
 * This implementation of source_manager_backend is backed by a std::map of file
 * names to their contents.
 */
class in_memory_source_manager_backend final : public source_manager_backend {
 public:
  using map = std::map<std::string, std::string, std::less<>>;

  std::optional<std::vector<char>> read_file(std::string_view path) final;
  bool exists(const std::filesystem::path& path) final;

  explicit in_memory_source_manager_backend(map files_by_path)
      : files_by_path_(std::move(files_by_path)) {}

 private:
  map files_by_path_;
};

} // namespace apache::thrift::compiler
