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

#include <boost/algorithm/string.hpp>
#include <fmt/core.h>
#include <fmt/format.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace codemod {

inline constexpr auto kDefaultDomain = "meta.com";

class package_name_generator {
 public:
  explicit package_name_generator(const std::string& ns) {
    split(identifiers_, ns, boost::algorithm::is_any_of("."));
    create_path_and_domain();
  }

  std::string generate(
      const std::string& default_domain = kDefaultDomain) const {
    auto domain = get_domain();
    if (domain.empty()) {
      domain = default_domain.empty() ? kDefaultDomain : default_domain;
    }
    return from_path_and_domain(get_path(), domain);
  }

  std::string get_path() const {
    return fmt::to_string(fmt::join(path_.begin(), path_.end(), "/"));
  }

  std::string get_domain() const {
    return fmt::to_string(fmt::join(domain_.begin(), domain_.end(), "."));
  }

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

 private:
  std::vector<std::string> identifiers_;
  std::vector<std::string> path_;
  std::vector<std::string> domain_;

  static inline const std::map<std::string, std::string> kPotentialDomainNames =
      {{"facebook", "com"},
       {"instagram", "com"},
       {"meta", "com"},
       {"apache", "org"}};

  void create_path_and_domain() {
    path_ = identifiers_;
    // Check if any potential domain name is present
    auto iter =
        std::find_if(path_.begin(), path_.end(), [&](const std::string& str) {
          return kPotentialDomainNames.find(str) != kPotentialDomainNames.end();
        });

    if (iter != path_.end()) {
      auto tld = kPotentialDomainNames.at(*iter);

      // Check if TLD is already present in path
      auto tld_iter = std::find(path_.begin(), iter, tld);
      domain_ = {path_.begin(), iter + 1};
      // If TLD is not present, add it
      if (tld_iter == iter) {
        domain_.insert(domain_.begin(), kPotentialDomainNames.at(*iter));
      }
      // Reverse the domain
      std::reverse(domain_.begin(), domain_.end());
      // Remove domain from the path
      path_.erase(path_.begin(), iter + 1);
    }
  }
};
} // namespace codemod
} // namespace compiler
} // namespace thrift
} // namespace apache
