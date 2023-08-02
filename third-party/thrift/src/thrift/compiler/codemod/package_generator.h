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

#include <string>

#include <fmt/core.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace codemod {

inline constexpr auto kDefaultDomain = "meta.com";

class package_name_generator {
 public:
  static std::string from_file_path(const std::string& path) {
    auto dot = path.find_last_of('.');
    auto slash = path.find_first_of('/');
    if (slash == std::string::npos) {
      slash = 0;
    } else {
      slash++;
    }
    if (dot != std::string::npos && slash < dot) {
      return from_path_and_domain(
          path.substr(slash, dot - slash), kDefaultDomain);
    }
    return from_path_and_domain(path, kDefaultDomain);
  }

  static std::string from_path_and_domain(
      const std::string& path, const std::string& domain) {
    return fmt::format("{}/{}", domain, path);
  }
};
} // namespace codemod
} // namespace compiler
} // namespace thrift
} // namespace apache
