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

#include <thrift/compiler/detail/system.h>
#include <thrift/compiler/source_location.h>

#include <fmt/core.h>

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

namespace apache::thrift::compiler {

namespace {

struct read_result {
  size_t count = 0;
  int error_code = 0;
};

class file {
 private:
  FILE* file_;

 public:
  file(const char* filename, const char* mode) : file_(fopen(filename, mode)) {}
  ~file() {
    if (file_) {
      int result = fclose(file_);
      (void)result;
      assert(result == 0);
    }
  }

  file(const file&) = delete;
  void operator=(const file&) = delete;

  explicit operator bool() const { return file_ != nullptr; }

  read_result read(char* buffer, size_t size) {
    size_t result = fread(buffer, 1, size, file_);
    int error_code = errno;
    return read_result{result, result == 0 && ferror(file_) ? error_code : 0};
  }
};

std::vector<uint_least32_t> get_line_offsets(std::string_view sv) {
  std::vector<uint_least32_t> line_offsets;
  line_offsets.push_back(0);

  const char* const begin = sv.data();
  const char* const end = begin + sv.size() - 1;
  const char* ptr = begin;
  for (;;) {
    ptr = static_cast<const char*>(memchr(ptr, '\n', end - ptr));
    if (ptr == nullptr) {
      break;
    }
    ++ptr;
    line_offsets.push_back(ptr - begin);
  }
  return line_offsets;
}

} // namespace

source source_manager::add_source(
    const std::string& file_name, std::vector<char> text) {
  assert(text.back() == '\0');
  std::string_view sv(text.data(), text.size());
  sources_.push_back(
      source_info{file_name, std::move(text), get_line_offsets(sv)});
  return {/* .start = */
          source_location(/* source_id= */ sources_.size(), /** offset= */ 0),
          /* .text = */ sv};
}

std::string source_manager::get_file_path(const std::string& file_name) const {
  if (file_source_map_.find(file_name) != file_source_map_.end()) {
    return file_name;
  }
  return std::filesystem::absolute(file_name).string();
}

std::optional<source> source_manager::get_file(const std::string& file_name) {
  if (auto source = file_source_map_.find(file_name);
      source != file_source_map_.end()) {
    return source->second;
  }

  std::string_view path;
  if (auto itr = found_includes_.find(file_name);
      itr != found_includes_.end()) {
    path = itr->second;
  } else {
    path = file_name;
  }

  if (auto source = file_source_map_.find(std::string(path));
      source != file_source_map_.end()) {
    return source->second;
  }

  std::string absPath;
  if (detail::platform_is_windows()) {
    // Without the "\\?\" prefix, path in Windows can not exceed 260 characters.
    // https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
    constexpr auto kPrefix = R"(\\?\)";
    if (path.substr(0, std::strlen(kPrefix)) != kPrefix) {
      absPath = kPrefix + std::filesystem::absolute(path).string();
      path = absPath;
    }
  }

  // Read the file.
  auto f = file(path.data(), "rb");
  if (!f) {
    return {};
  }
  char buffer[4096];
  auto text = std::vector<char>();
  for (;;) {
    auto result = f.read(buffer, sizeof(buffer));
    if (result.count == 0) {
      if (result.error_code == 0) {
        break;
      }
      return {};
    }
    text.insert(text.end(), buffer, buffer + result.count);
  }
  text.push_back('\0');

  auto source = add_source(file_name, std::move(text));
  file_source_map_.emplace(file_name, source);
  return source;
}

source source_manager::add_virtual_file(
    const std::string& file_name, const std::string& src) {
  if (file_source_map_.find(file_name) != file_source_map_.end()) {
    throw std::runtime_error(std::string("file already added: ") + file_name);
  }
  const char* start = src.c_str();
  auto source =
      add_source(file_name, std::vector<char>(start, start + src.size() + 1));
  file_source_map_.emplace(file_name, source);
  return source;
}

const char* source_manager::get_text(source_location loc) const {
  const source_info* source = get_source(loc.source_id_);
  return source ? &source->text[loc.offset_] : nullptr;
}

std::string_view source_manager::get_text_range(
    const source_range& range) const {
  assert(range.begin.source_id_ == range.end.source_id_);
  const char* first = get_text(range.begin);
  assert(first != nullptr);
  const char* last = get_text(range.end);
  assert(last != nullptr);
  return std::string_view(first, /* count= */ last - first);
}

resolved_location::resolved_location(
    source_location loc, const source_manager& sm) {
  if (loc == source_location()) {
    throw std::invalid_argument("empty source location");
  }
  const source_manager::source_info* source = sm.get_source(loc.source_id_);
  if (!source) {
    throw std::invalid_argument("invalid source location");
  }

  file_name_ = source->file_name.c_str();
  const std::vector<uint_least32_t>& line_offsets = source->line_offsets;
  auto it =
      std::upper_bound(line_offsets.begin(), line_offsets.end(), loc.offset_);
  line_ = it != line_offsets.end() ? it - line_offsets.begin()
                                   : line_offsets.size();
  column_ = loc.offset_ - line_offsets[line_ - 1] + 1;
}

source_manager::path_or_error source_manager::find_include_file(
    const std::string& filename,
    const std::string& parent_path,
    const std::vector<std::string>& search_paths) {
  if (auto itr = found_includes_.find(filename); itr != found_includes_.end()) {
    return source_manager::path_or_error{std::in_place_index<0>, itr->second};
  }

  auto found = [&](std::string path) {
    found_includes_[filename] = path;
    return source_manager::path_or_error{
        std::in_place_index<0>, std::move(path)};
  };

  if (file_source_map_.find(filename) != file_source_map_.end()) {
    return found(filename);
  }

  // Absolute path? Just try that.
  std::filesystem::path path(filename);
  if (path.has_root_directory()) {
    try {
      return found(std::filesystem::canonical(path).string());
    } catch (const std::filesystem::filesystem_error& e) {
      return source_manager::path_or_error{
          std::in_place_index<1>,
          fmt::format(
              "Could not find file: {}. Error: {}", filename, e.what())};
    }
  }

  // Relative path, start searching
  // new search path with current dir global
  std::vector<std::string> sp = search_paths;
  auto itr = found_includes_.find(parent_path);
  const std::string& resolved_parent_path =
      itr != found_includes_.end() ? itr->second : parent_path;
  auto dir = std::filesystem::path(resolved_parent_path).parent_path().string();
  dir = dir.empty() ? "." : dir;
  sp.insert(sp.begin(), std::move(dir));
  // Iterate through paths.
  std::vector<std::string>::iterator it;
  for (it = sp.begin(); it != sp.end(); it++) {
    std::filesystem::path sfilename = filename;
    if ((*it) != "." && (*it) != "") {
      sfilename = std::filesystem::path(*(it)) / filename;
    }
    if (std::filesystem::exists(sfilename) ||
        file_source_map_.find(sfilename.string()) != file_source_map_.end()) {
      return found(sfilename.string());
    }
#ifdef _WIN32
    // On Windows, handle files found at potentially long paths.
    sfilename = R"(\\?\)" +
        std::filesystem::absolute(sfilename)
            .make_preferred()
            .lexically_normal()
            .string();
    if (std::filesystem::exists(sfilename)) {
      return found(sfilename.string());
    }
#endif
  }
  // File was not found.
  return source_manager::path_or_error{
      std::in_place_index<1>,
      fmt::format("Could not find include file {}", filename)};
}

std::optional<std::string> source_manager::found_include_file(
    const std::string& filename) const {
  if (auto itr = found_includes_.find(filename); itr != found_includes_.end()) {
    return itr->second;
  }
  return {};
}

} // namespace apache::thrift::compiler
