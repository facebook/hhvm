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

#include <map>
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

class package_name_generator_util {
 public:
  explicit package_name_generator_util(
      std::vector<package_name_generator> pkg_generators)
      : pkg_generators_(std::move(pkg_generators)) {
    for (const auto& generator : pkg_generators_) {
      const auto domain = generator.get_domain();
      if (!domain.empty()) {
        any_domain_ = domain;
        break;
      }
    }
  }

  static package_name_generator_util from_namespaces(
      const std::map<std::string, std::string>& namespaces) {
    std::vector<package_name_generator> pkg_generators;
    pkg_generators.reserve(namespaces.size());
    for (const auto& [_, ns] : namespaces) {
      pkg_generators.emplace_back(ns);
    }
    return package_name_generator_util(std::move(pkg_generators));
  }

  std::string find_common_package() const {
    struct freq_and_domain {
      int freq;
      std::string domain;
    };
    std::map<std::string, freq_and_domain> namespace_path_freq_map;
    std::string most_freq_ns_path;
    auto process_ns_path = [&](const std::string& cur_ns_path,
                               const std::string& cur_ns_domain) {
      if (cur_ns_path.empty()) {
        return;
      }

      // Only use namespace path for identifying common package
      // so that if 2 or more namespaces have same path
      // and different or no domains,
      // then we still consider them as same.
      if (namespace_path_freq_map.find(cur_ns_path) ==
          namespace_path_freq_map.end()) {
        namespace_path_freq_map[cur_ns_path] = {1, cur_ns_domain};
        return;
      }
      auto& [cur_ns_path_freq, cur_ns_path_domain] =
          namespace_path_freq_map[cur_ns_path];
      cur_ns_path_freq++;
      // If there is a domain associated with this path,
      // then we should use that instead of default one.
      if (!cur_ns_domain.empty()) {
        cur_ns_path_domain = cur_ns_domain;
      }

      auto& [max_freq, _] = namespace_path_freq_map[most_freq_ns_path];

      // If the current path has the highest frequency, update it.
      // Alternatively, if the frequency is equal to the current highest but the
      // path is longer, prioritize the longer path as it is more likely to be
      // unique
      if (cur_ns_path_freq > max_freq ||
          (cur_ns_path_freq == max_freq &&
           cur_ns_path.length() > most_freq_ns_path.length())) {
        most_freq_ns_path = cur_ns_path;
      }
    };

    for (auto& generator : pkg_generators_) {
      process_ns_path(generator.get_path(), generator.get_domain());
    }

    if (most_freq_ns_path.empty()) {
      return "";
    }

    auto domain = namespace_path_freq_map.at(most_freq_ns_path).domain;
    /*
     * If the guessed package doesn't have a domain
     * but there is domain present in one of the namespaces,
     * then use that domain instead of the default domain
     */
    if (domain.empty()) {
      domain = any_domain_;
    }
    return package_name_generator::from_path_and_domain(
        most_freq_ns_path, domain);
  }

 private:
  std::vector<package_name_generator> pkg_generators_;
  std::string any_domain_ = kDefaultDomain;
};
} // namespace codemod
} // namespace compiler
} // namespace thrift
} // namespace apache
