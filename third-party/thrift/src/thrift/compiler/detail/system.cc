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

#include <functional>

#include <boost/algorithm/string/replace.hpp>

namespace apache::thrift::compiler::detail {

std::filesystem::path make_abs_path(
    const std::filesystem::path& base_path, const std::filesystem::path& path) {
  auto abs_path = std::invoke([&] {
    if (path.empty()) {
      return base_path;
    } else if (path.is_absolute()) {
      return path;
    } else if (!path.root_directory().empty()) {
      // to help on windows with path relative but starting with `\\`
      return base_path / path.relative_path();
    } else {
      return base_path / path;
    }
  });
  if (platform_is_windows()) {
    return format_abs_path(abs_path.lexically_normal().string());
  }
  return abs_path;
}

std::filesystem::path format_abs_path(std::string_view path) {
  auto abs_path = std::filesystem::path{path};
  if (platform_is_windows()) {
    // Handles long paths on windows.
    // https://www.boost.org/doc/libs/1_69_0/libs/filesystem/doc/reference.html#long-path-warning
    static constexpr auto kExtendedLengthPathPrefix = L"\\\\?\\";
    if (!abs_path.wstring().starts_with(kExtendedLengthPathPrefix)) {
      auto native_path = abs_path.make_preferred().wstring();
      // At this point the path may have a mix of '\\' and '\' separators.
      boost::algorithm::replace_all(native_path, L"\\\\", L"\\");
      abs_path = kExtendedLengthPathPrefix + native_path;
    }
  }
  return abs_path;
}

} // namespace apache::thrift::compiler::detail
