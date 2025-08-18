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
#include <fmt/std.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cerrno>
#include <cstring>
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

class default_source_manager_backend final : public source_manager_backend {
 public:
  std::optional<std::vector<char>> read_file(std::string_view path) final {
    std::string abs_path;
    if constexpr (detail::platform_is_windows()) {
      // Without the "\\?\" prefix, path in Windows can not exceed 260
      // characters.
      // https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
      constexpr auto unc_prefix = R"(\\?\)";
      if (path.substr(0, std::strlen(unc_prefix)) != unc_prefix) {
        abs_path = unc_prefix + std::filesystem::absolute(path).string();
        path = abs_path;
      }
    } else {
      // This ensures that the path is NUL-terminated, which is required by the
      // fopen call that follows.
      abs_path = path;
      path = abs_path;
    }

    // Read the file.
    auto f = file(path.data(), "rb");
    if (!f) {
      return {};
    }
    std::array<char, 4096> buffer;
    auto text = std::vector<char>();
    for (;;) {
      auto result = f.read(buffer.data(), buffer.size());
      if (result.count == 0) {
        if (result.error_code == 0) {
          break;
        }
        return {};
      }
      text.insert(text.end(), buffer.data(), buffer.data() + result.count);
    }
    text.push_back('\0');
    return text;
  }

  bool exists(const std::filesystem::path& path) final {
    return std::filesystem::exists(path);
  }
};

/**
 * Returns the parent directory of the given `target_path`.
 *
 * @param resolved_paths Cache of previously resolved paths.
 */
std::filesystem::path resolve_parent_directory(
    std::string_view target_path,
    const std::map<std::string, std::string, std::less<>>& resolved_paths) {
  if (auto itr = resolved_paths.find(target_path);
      itr != resolved_paths.end()) {
    target_path = itr->second;
  }

  std::filesystem::path parent_dir =
      std::filesystem::path(target_path).parent_path();

  return parent_dir.empty() ? std::filesystem::path(".") : parent_dir;
}

} // namespace

source_manager::source_manager()
    : backend_(std::make_unique<default_source_manager_backend>()) {}

source_view source_manager::add_source(
    std::string_view file_name, std::vector<char> text) {
  assert(text.back() == '\0');
  std::string_view sv(text.data(), text.size());
  sources_.push_back(source_info{
      std::string(file_name), std::move(text), get_line_offsets(sv)});
  return source_view{
      /* .start = */
      source_location(/* source_id= */ sources_.size(), /* offset= */ 0),
      /* .text = */ sv};
}

std::string source_manager::get_file_path(std::string_view file_name) const {
  if (file_source_map_.find(file_name) != file_source_map_.end()) {
    return std::string(file_name);
  }
  return std::filesystem::absolute(file_name).string();
}

std::optional<source_view> source_manager::get_file(
    std::string_view file_name) {
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

  if (auto source = file_source_map_.find(path);
      source != file_source_map_.end()) {
    return source->second;
  }

  if (backend_ == nullptr) {
    return std::nullopt;
  }
  std::optional<std::vector<char>> text = backend_->read_file(path);
  if (!text.has_value()) {
    return std::nullopt;
  }
  auto source = add_source(file_name, std::move(text).value());
  file_source_map_.emplace(file_name, source);
  return source;
}

source_view source_manager::add_virtual_file(
    std::string_view file_name, std::string_view src) {
  if (file_source_map_.find(file_name) != file_source_map_.end()) {
    throw std::runtime_error(fmt::format("file already added: {}", file_name));
  }
  const char* start = src.data();
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
  return std::string_view(first, /* count */ last - first);
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

std::optional<std::filesystem::path> source_manager::try_search_path(
    const std::filesystem::path& include_path,
    const std::filesystem::path& search_path) {
  std::filesystem::path sfilename = include_path;
  if (search_path != "." && search_path != "") {
    sfilename = std::filesystem::path(search_path) / include_path;
  }
  if (path_exists_in_backend(sfilename) ||
      file_source_map_.find(sfilename.string()) != file_source_map_.end()) {
    return sfilename;
  }
#ifdef _WIN32
  // On Windows, handle files found at potentially long paths.
  sfilename = R"(\\?\)" +
      std::filesystem::absolute(sfilename)
          .make_preferred()
          .lexically_normal()
          .string();
  if (path_exists_in_backend(sfilename)) {
    return sfilename;
  }
#endif
  return std::nullopt;
}

bool source_manager::path_exists_in_backend(
    const std::filesystem::path& path) const {
  return backend_ == nullptr ? false : backend_->exists(path);
}

source_manager::path_or_error source_manager::find_include_file(
    std::string_view include_file_path,
    const std::vector<std::string>& search_paths,
    std::optional<std::string_view> program_path) {
  // If the given `include_file_path` has already been resolved, return the
  // known path.
  if (auto itr = found_includes_.find(include_file_path);
      itr != found_includes_.end()) {
    return source_manager::path_or_error{std::in_place_index<0>, itr->second};
  }

  // Convenience callback to use when the path the the include has been
  // succcessfully resolved: updates the set of known paths and returns a
  // successful resutl with the resolved path.
  auto new_found_include = [&](std::string path) {
    found_includes_[std::string(include_file_path)] = path;
    return source_manager::path_or_error{
        std::in_place_index<0>, std::move(path)};
  };

  // The included_file_path is already known as a source file: return it
  // directly.
  if (file_source_map_.find(include_file_path) != file_source_map_.end()) {
    return new_found_include(std::string(include_file_path));
  }

  // Absolute path? Just try that.
  std::filesystem::path path(include_file_path);
  if (path.has_root_directory()) {
    try {
      return new_found_include(std::filesystem::canonical(path).string());
    } catch (const std::filesystem::filesystem_error& e) {
      return source_manager::path_or_error{
          std::in_place_index<1>,
          fmt::format(
              "Could not find file: {}. Error: {}",
              include_file_path,
              e.what())};
    }
  }

  if (program_path.has_value()) {
    // Look for the given include path relative to parent directory
    const std::filesystem::path parent_dir =
        resolve_parent_directory(program_path.value(), found_includes_);
    if (auto foundPathInParentDir =
            try_search_path(include_file_path, parent_dir)) {
      return new_found_include(foundPathInParentDir->string());
    }
  }

  // Iterate through `search_paths`.
  for (const std::string& search_path : search_paths) {
    if (auto foundPath = try_search_path(include_file_path, search_path)) {
      return new_found_include(foundPath->string());
    }
  }
  // File was not found.
  return source_manager::path_or_error{
      std::in_place_index<1>,
      fmt::format("Could not find include file {}", include_file_path)};
}

std::optional<std::string> source_manager::found_include_file(
    std::string_view filename) const {
  if (auto itr = found_includes_.find(filename); itr != found_includes_.end()) {
    return itr->second;
  }
  return {};
}

std::optional<std::vector<char>> in_memory_source_manager_backend::read_file(
    std::string_view path) {
  auto found = files_by_path_.find(path);
  if (found == files_by_path_.end()) {
    return std::nullopt;
  }
  const auto& [_, source_code] = *found;
  std::vector<char> result;
  result.reserve(source_code.size() + 1);
  result.insert(result.end(), source_code.begin(), source_code.end());
  result.push_back('\0');
  return result;
}

bool in_memory_source_manager_backend::exists(
    const std::filesystem::path& path) {
  // The in-memory source manager backend only supports generic path format
  // since it's intended to be platform-agnostic.
  return files_by_path_.find(path.generic_string()) != files_by_path_.end();
}

} // namespace apache::thrift::compiler
