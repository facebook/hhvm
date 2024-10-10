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

#include <filesystem>
#include <string>
#include <string_view>

namespace apache::thrift::compiler::detail {

// Returns true on a Windows platform, false elsewhere.
constexpr bool platform_is_windows() {
#ifdef _WIN32
  return true;
#else
  return false;
#endif
}

/**
 * Appends `path` to `base_path` to create an absolute path.
 *
 * @param base_path The base path to append `path` to.
 * @param path The path to append to the `base_path`.
 * @return The absolute path created by appending `path` to `base_path`.
 */
std::filesystem::path make_abs_path(
    const std::filesystem::path& base_path, const std::filesystem::path& path);

/**
 * Formats `path` to include extended length prefix on windows
 *
 * @param path The path to prepend the prefix to (must be an absolute path)
 * @return On Windows, the std::filesystem::path created by prepending "\\?\"
 * to `path` if not already so. Otherwise, `std::filesystem::path{path}`.
 */
std::filesystem::path format_abs_path(std::string_view path);

} // namespace apache::thrift::compiler::detail
