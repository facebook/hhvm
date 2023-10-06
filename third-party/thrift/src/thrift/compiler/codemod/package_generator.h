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
#include <re2/re2.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace codemod {

inline constexpr auto kDefaultDomain = "meta.com";

class package_name_generator {
 public:
  explicit package_name_generator(
      const std::string& language, const std::string& ns)
      : language_(language) {
    std::vector<std::string> identifiers;
    split(identifiers, ns, boost::algorithm::is_any_of("."));
    create_path_and_domain(std::move(identifiers));
  }

  explicit package_name_generator(
      std::vector<std::string> path, std::vector<std::string> domain = {})
      : path_(std::move(path)), domain_(std::move(domain)) {}

  explicit package_name_generator(std::string file_path) {
    std::vector<std::string> identifiers;
    split(identifiers, file_path, boost::algorithm::is_any_of("/"));
    create_path_and_domain(std::move(identifiers));
  }

  const std::vector<std::string>& get_path_components() const { return path_; }

  std::string generate(
      const std::string& default_domain = kDefaultDomain) const {
    auto domain = get_domain();
    if (domain.empty()) {
      domain = default_domain.empty() ? kDefaultDomain : default_domain;
    }
    return from_path_and_domain(get_path(path_), domain);
  }

  std::string get_path(bool use_modified_path = false) const {
    return get_path(use_modified_path ? get_modified_path() : path_);
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
      return package_name_generator(path.substr(slash, dot - slash))
          .generate(kDefaultDomain);
    }
    return package_name_generator(path).generate(kDefaultDomain);
  }

  static std::string from_path_and_domain(
      const std::string& path, const std::string& domain) {
    return fmt::format("{}/{}", domain, path);
  }

 private:
  std::string language_;
  std::vector<std::string> path_;
  std::vector<std::string> domain_;

  static inline const std::map<std::string, std::string> kPotentialDomainNames =
      {{"facebook", "com"},
       {"instagram", "com"},
       {"meta", "com"},
       {"apache", "org"}};

  // Regex to find respective language specific
  // substrings in the identifier
  static inline const std::map<std::string, std::string> kLanguageIdentifiers =
      {
          {"cpp", "cpp1|cpp2|cpp"},
          {"cpp2", "cpp1|cpp2|cpp"},
          {"java", "java2|java|swift|java_swift"},
          {"java2", "java2|java|swift|java_swift"},
          {"java.swift", "java2|java|java_swift"},
          {"py", "py|py3|python|asyncio|py3_asyncio"},
          {"py3", "py|py3|python|asyncio|py3_asyncio"},
          {"python", "py|py3|python|asyncio|py3_asyncio"},
          {"py.asyncio", "py|py3|python|asyncio|py3_asyncio"},
          {"php", "php"},
          {"hack", "hack"},
  };

  std::string replace_language_name(std::string identifier) const {
    std::string pattern;
    if (kLanguageIdentifiers.find(language_) == kLanguageIdentifiers.end()) {
      pattern = language_;
    } else {
      pattern = kLanguageIdentifiers.at(language_);
    }
    // Only replace if the pattern is entire word
    // i.e. _<pattern>_ OR <pattern>_ OR _<pattern> OR <pattern>.
    // Doesn't match if the language name is part of other word
    // Eg: `hack` shouldn't be replaced in "hacker_php"
    re2::RE2 re("(_|^)(" + pattern + ")(_|$)");
    re2::RE2::GlobalReplace(&identifier, re, "\\3");
    if (identifier.find_first_of('_') == 0) {
      return identifier.substr(1);
    }
    return identifier;
  }

  // Remove identifer or substrings specific to the language
  std::vector<std::string> get_modified_path() const {
    bool found_modified_path = false;
    std::vector<std::string> modified_path;
    for (auto& identifier : path_) {
      auto modified_identifier = replace_language_name(identifier);
      if (!modified_identifier.empty()) {
        modified_path.push_back(modified_identifier);
      }
      if (!found_modified_path && modified_identifier != identifier) {
        found_modified_path = true;
      }
    }
    // If no modification was found return empty vector.
    // This can happen when there are no language specific identifiers present
    if (!found_modified_path) {
      return {};
    } else {
      return modified_path;
    }
  }

  std::string get_path(const std::vector<std::string>& path) const {
    return fmt::to_string(fmt::join(path.begin(), path.end(), "/"));
  }

  std::string to_snake_case(std::string input) {
    static const re2::RE2 lowercase = "([a-z0-9])([A-Z])";
    /*
     * Match where a lowercase letter or a digit is followed
     * by an uppercase letter.
     *
     * BazFOOBar => Baz_FOOBar
     * FOO2Bar   => FOO2_Bar
     * FooBar    => Foo_Bar
     * BarZ      => Bar_Z
     */
    re2::RE2::GlobalReplace(&input, lowercase, "\\1_\\2");

    /*
     * Match where an uppercase letter or a digit is followed
     * by an uppercase letter and a lowercase letter.
     *
     * Baz_FOOBar => Baz_FOO_Bar
     */
    re2::RE2 uppercase = "([A-Z0-9])([A-Z][a-z])";
    re2::RE2::GlobalReplace(&input, uppercase, "\\1_\\2");

    // Convert the string to lowercase.
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    return input;
  }

  void create_path_and_domain(std::vector<std::string> identifiers) {
    path_ = identifiers;
    // Convert all characters to lowercase
    for (auto& identifier : path_) {
      identifier = to_snake_case(identifier);
    }
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
    for (const auto& [lang, ns] : namespaces) {
      pkg_generators.emplace_back(lang, ns);
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
      const auto& cur_domain = generator.get_domain();

      /*
       * Sometimes namespaces have language name in them,
       * because of which common package cannot be identified.
       *
       * While finding the common package, also try with a modified path
       * by removing the respective language name from the identifiers.
       *
       * Accounts for the use case where a language name
       * is actually part of the namespace.
       * That is for c++ namespace, only remove c++ related substrings.
       * Eg: namespace cpp2 "cpp.hack.annotation" => "hack.annotation"
       */
      for (auto use_modified_path : {true, false}) {
        process_ns_path(generator.get_path(use_modified_path), cur_domain);
      }
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

  std::string get_package_from_common_identifiers() const {
    auto common_identifiers = find_matching_identifiers(
        pkg_generators_.at(0).get_path_components(),
        pkg_generators_.at(1).get_path_components());
    for (size_t i = 2; i < pkg_generators_.size(); i++) {
      common_identifiers = find_matching_identifiers(
          common_identifiers, pkg_generators_.at(i).get_path_components());
      // If there are no common identifiers, then return empty string
      if (common_identifiers.empty()) {
        return "";
      }
    }

    // When guessing package from common identifiers,
    // minimum path length is required.
    if (common_identifiers.size() < 2) {
      return "";
    }

    return package_name_generator(
               get_ordered_common_identifiers(common_identifiers))
        .generate(any_domain_);
  }

  std::string get_longest_package() {
    std::string longest_package;
    for (auto& generator : pkg_generators_) {
      auto pkg = generator.generate(any_domain_);
      if (pkg.length() > longest_package.length()) {
        longest_package = pkg;
      }
    }
    return longest_package;
  }

 private:
  std::vector<package_name_generator> pkg_generators_;
  std::string any_domain_ = kDefaultDomain;

  std::vector<std::string> find_matching_identifiers(
      std::vector<std::string> ns1, std::vector<std::string> ns2) const {
    std::vector<std::string> matches;

    // `set_intersection` requires sorted lists
    std::sort(ns1.begin(), ns1.end());
    std::sort(ns2.begin(), ns2.end());

    std::set_intersection(
        ns1.begin(),
        ns1.end(),
        ns2.begin(),
        ns2.end(),
        std::back_inserter(matches));

    return matches;
  }

  /*
   * The sequence of identifiers as they are in the source namespace is lost
   * while generating common identifiers. Hence we need to re-order them.
   */
  std::vector<std::string> get_ordered_common_identifiers(
      const std::vector<std::string>& common_identifiers) const {
    std::vector<std::string> ordered_identifiers;

    // Common identifiers are present in all the namespaces,
    // so use any namespace to derive the original order.

    for (auto& identifier : pkg_generators_.begin()->get_path_components()) {
      if (std::find(
              common_identifiers.begin(),
              common_identifiers.end(),
              identifier) != common_identifiers.end()) {
        ordered_identifiers.push_back(identifier);
      }
    }
    return ordered_identifiers;
  }
};
} // namespace codemod
} // namespace compiler
} // namespace thrift
} // namespace apache
