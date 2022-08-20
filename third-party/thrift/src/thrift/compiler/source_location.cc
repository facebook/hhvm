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

#include <thrift/compiler/source_location.h>

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <stdexcept>

namespace apache {
namespace thrift {
namespace compiler {

namespace {

class file {
 private:
  FILE* file_;

 public:
  file(const char* filename, const char* mode) : file_(fopen(filename, mode)) {
    if (!file_) {
      throw std::runtime_error(std::string("failed to open file: ") + filename);
    }
  }
  ~file() {
    int result = fclose(file_);
    (void)result;
    assert(result == 0);
  }

  file(const file&) = delete;
  void operator=(const file&) = delete;

  size_t read(char* buffer, size_t size) {
    size_t result = fread(buffer, 1, size, file_);
    int error_code = errno;
    if (result == 0 && ferror(file_)) {
      throw std::runtime_error(
          fmt::format("error reading from file, error code = {}", error_code));
    }
    return result;
  }
};

} // namespace

source source_manager::add_source(
    std::string file_name, std::vector<char> text) {
  assert(text.back() == '\0');
  auto sv = fmt::string_view(text.data(), text.size());
  auto src = source_info{file_name, std::move(text), {}};

  src.line_offsets.push_back(0);
  auto begin = sv.data(), end = begin + sv.size() - 1;
  const char* ptr = begin;
  for (;;) {
    ptr = static_cast<const char*>(memchr(ptr, '\n', end - ptr));
    if (!ptr) {
      break;
    }
    ++ptr;
    src.line_offsets.push_back(ptr - begin);
  }

  sources_.push_back(std::move(src));
  return {source_location(sources_.size(), 0), sv};
}

source source_manager::add_file(std::string file_name) {
  auto f = file(file_name.c_str(), "rb");
  char buffer[4096];
  auto text = std::vector<char>();
  while (size_t count = f.read(buffer, sizeof(buffer))) {
    text.insert(text.end(), buffer, buffer + count);
  }
  text.push_back('\0');
  return add_source(std::move(file_name), std::move(text));
}

source source_manager::add_string(std::string file_name, std::string src) {
  const char* start = src.c_str();
  return add_source(
      std::move(file_name), std::vector<char>(start, start + src.size() + 1));
}

const char* source_manager::get_text(source_location loc) const {
  const source_info* source = get_source(loc.source_id_);
  return source ? &source->text[loc.offset_] : nullptr;
}

resolved_location::resolved_location(
    source_location loc, const source_manager& sm) {
  const source_manager::source_info* source = sm.get_source(loc.source_id_);
  assert(source);
  file_name_ = source->file_name.c_str();
  const std::vector<uint_least32_t>& line_offsets = source->line_offsets;
  auto it =
      std::upper_bound(line_offsets.begin(), line_offsets.end(), loc.offset_);
  line_ = it != line_offsets.end() ? it - line_offsets.begin()
                                   : line_offsets.size();
  column_ = loc.offset_ - line_offsets[line_ - 1] + 1;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
