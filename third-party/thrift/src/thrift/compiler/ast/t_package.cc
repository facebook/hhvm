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
#include <thrift/common/universal_name.h>

namespace apache::thrift::compiler {
namespace {

const char DOMAIN_DELIM[] = ".";
const char PATH_DELIM[] = "/";

std::string gen_prefix(
    const std::vector<std::string>& domain,
    const std::vector<std::string>& path) {
  std::string result;
  const auto* delim = "";
  for (const auto& label : domain) {
    result += delim;
    result += label;
    delim = DOMAIN_DELIM;
  }
  delim = PATH_DELIM;
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
  detail::check_univeral_name_domain(labels);
  return labels;
}

} // namespace

t_package::t_package(std::string name)
    : uri_prefix_(std::move(name)), explicit_(true) {
  boost::algorithm::split(
      path_, uri_prefix_, [](auto ch) { return ch == PATH_DELIM[0]; });
  if (path_.size() < 2) {
    throw std::invalid_argument("invalid package name");
  }
  domain_ = parse_domain(path_.front());
  path_.erase(path_.begin());
  detail::check_universal_name_path(path_);
  uri_prefix_ += "/";
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
  detail::check_univeral_name_domain(domain_);
  detail::check_universal_name_path(path_);
  uri_prefix_ = gen_prefix(domain_, path_);
}

} // namespace apache::thrift::compiler
