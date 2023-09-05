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

#include <stdint.h>

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/optional.hpp>

namespace apache {
namespace thrift {
namespace compiler {

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
  source_location begin;
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
struct source {
  source_location start; // The source start location.
  std::string_view text; // The source text including a terminating '\0'.
};

// A source manager that caches sources in memory, loads files and enables
// resolution of offset-based source locations into file names, lines and
// columns.
class source_manager {
 private:
  struct source_info {
    std::string file_name;
    std::vector<char> text;
    std::vector<uint_least32_t> line_offsets;
  };
  // This is a deque to make sure that file_name is not reallocated when
  // sources_ grows.
  std::deque<source_info> sources_;

  std::unordered_map<std::string, source> file_source_map_;

  const source_info* get_source(uint_least32_t source_id) const {
    return source_id > 0 && source_id <= sources_.size()
        ? &sources_[source_id - 1]
        : nullptr;
  }

  friend class resolved_location;

  source add_source(const std::string& file_name, std::vector<char> text);

 public:
  // Loads a file and returns a source object representing its content.
  // The file can be a real file or a virtual one previously registered with
  // add_virtual_file.
  // Returns an empty optional if opening or reading the file fails.
  boost::optional<source> get_file(const std::string& file_name);

  // Adds a virtual file with the specified name and content.
  source add_virtual_file(const std::string& file_name, const std::string& src);

  // Returns the start location of a source containing the specified location.
  // It is a member function in case we add clang-like compression of locations.
  source_location get_source_start(source_location loc) const {
    return {loc.source_id_, 0};
  }

  // Returns a pointer to the source text at the specified location or nullptr
  // if the location is invalid.
  const char* get_text(source_location loc) const;

  // Locates a filename among the include paths.
  static std::string find_include_file(
      const std::string& filename,
      const std::string& program_path,
      const std::vector<std::string>& search_paths);
};

} // namespace compiler
} // namespace thrift
} // namespace apache
