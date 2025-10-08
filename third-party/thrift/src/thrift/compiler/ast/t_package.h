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
#include <string_view>
#include <vector>

#include <thrift/compiler/ast/t_node.h>

namespace apache::thrift::compiler {

// The Thrift package that scopes IDL definitions.
class t_package : public t_node {
 public:
  t_package() = default;
  explicit t_package(std::string name);
  t_package(std::vector<std::string> domain, std::vector<std::string> path);
  // Explicit empty package (to hold annotations without doing anything else).
  struct explicitly_empty_tag {};
  explicit t_package(explicitly_empty_tag) : explicit_(true) {}

  // The domain 'labels'.
  const std::vector<std::string>& domain() const { return domain_; }
  // The path 'segments'
  const std::vector<std::string>& path() const { return path_; }

  // Returns the (scheme-less) uri for the given definition name, scoped by this
  // package.
  std::string get_uri(const std::string& name) const;

  // If the package has been set.
  bool is_explicit() const { return explicit_; }
  bool empty() const { return uri_prefix_.empty(); }

  // The raw package name.
  std::string_view name() const {
    std::string_view ret = uri_prefix_;
    if (!empty()) {
      ret.remove_suffix(1);
    }
    return ret;
  }

 private:
  std::string uri_prefix_;
  std::vector<std::string> domain_;
  std::vector<std::string> path_;
  bool explicit_ = false;

  friend bool operator==(const t_package& lhs, const t_package& rhs) {
    return lhs.uri_prefix_ == rhs.uri_prefix_;
  }
};

} // namespace apache::thrift::compiler
