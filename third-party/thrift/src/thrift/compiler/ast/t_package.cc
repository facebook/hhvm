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

#include <thrift/compiler/ast/t_package.h>

#include <boost/algorithm/string/split.hpp>

namespace apache::thrift::compiler {
namespace {

constexpr std::string_view DOMAIN_DELIM = ".";
constexpr std::string_view PATH_DELIM = "/";

std::string gen_prefix(
    const std::vector<std::string>& domain,
    const std::vector<std::string>& path) {
  std::string result;
  const auto* delim = "";
  for (const auto& label : domain) {
    result += delim;
    result += label;
    delim = DOMAIN_DELIM.data();
  }
  delim = PATH_DELIM.data();
  for (const auto& segment : path) {
    result += delim;
    result += segment;
  }
  result += delim;
  return result;
}

std::vector<std::string> parse_domain(const std::string& domain) {
  std::vector<std::string> labels;
  boost::algorithm::split(
      labels, domain, [](auto ch) { return ch == DOMAIN_DELIM[0]; });
  return labels;
}

} // namespace

t_package::t_package(std::string name)
    : uri_prefix_(std::move(name)), explicit_(true) {
  boost::algorithm::split(
      path_, uri_prefix_, [](auto ch) { return ch == PATH_DELIM[0]; });
  // The vector size is ideally length >=2 (domain and 1+ path segments) at this
  // point, but MIGHT be 1 (the entirety of `name`) if the package is malformed.
  // This will be validated in the standard_validator
  domain_ = parse_domain(path_.front());
  path_.erase(path_.begin());
  // There MIGHT already be a trailing slash in the input value (which is
  // invalid) but validation is done in the standard_validator, so only add one
  // if it's missing for now, so we don't generate unrelated noisy validations
  // for invalid URIs based off this package name, but just one for invalid
  // package
  if (!uri_prefix_.empty() && !uri_prefix_.ends_with('/')) {
    uri_prefix_ += '/';
  }
}

std::string t_package::get_uri(const std::string& name) const {
  if (empty()) {
    return {}; // Package is empty, so no URI.
  }
  return uri_prefix_ + name;
}

t_package::t_package(
    std::vector<std::string> domain, std::vector<std::string> path)
    : domain_(std::move(domain)), path_(std::move(path)), explicit_(true) {
  uri_prefix_ = gen_prefix(domain_, path_);
}

} // namespace apache::thrift::compiler
