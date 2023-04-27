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

#include <cctype>
#include <stdexcept>

#include <boost/algorithm/string/split.hpp>

namespace {

const char kDomainDelim[] = ".";
const char kPathDelim[] = "/";

std::string genPrefix(
    const std::vector<std::string>& domain,
    const std::vector<std::string>& path) {
  std::string result;
  const auto* delim = "";
  for (const auto& label : domain) {
    result += delim;
    result += label;
    delim = kDomainDelim;
  }
  delim = kPathDelim;
  for (const auto& segment : path) {
    result += delim;
    result += segment;
  }
  result += delim;
  return result;
}

void check(bool cond, const char* err) {
  if (!cond) {
    throw std::invalid_argument(err);
  }
}

bool isDomainChar(char c) {
  return std::isdigit(c) || std::islower(c) || c == '-';
}

bool isPathChar(char c) {
  return isDomainChar(c) || c == '_';
}

void checkDomainLabel(const std::string& label) {
  check(!label.empty(), "empty domain label");
  for (const auto& c : label) {
    check(isDomainChar(c), "invalid domain char");
  }
}

void checkDomain(const std::vector<std::string>& domain) {
  check(domain.size() >= 2, "not enough domain labels");
  for (const auto& label : domain) {
    checkDomainLabel(label);
  }
}

void checkPathSegment(const std::string& seg) {
  check(!seg.empty(), "empty path segment");
  for (const auto& c : seg) {
    check(isPathChar(c), "invalid path char");
  }
}

void checkPath(const std::vector<std::string>& path) {
  check(!path.empty(), "not enough path segments");
  for (const auto& seg : path) {
    checkPathSegment(seg);
  }
}

std::vector<std::string> parseDomain(const std::string& domain) {
  std::vector<std::string> labels;
  boost::algorithm::split(
      labels, domain, [](auto ch) { return ch == kDomainDelim[0]; });
  checkDomain(labels);
  return labels;
}

} // namespace

namespace apache {
namespace thrift {
namespace compiler {

t_package::t_package(std::string name) : uriPrefix_(std::move(name)) {
  boost::algorithm::split(
      path_, uriPrefix_, [](auto ch) { return ch == kPathDelim[0]; });
  check(path_.size() >= 2, "invalid package name");
  domain_ = parseDomain(path_.front());
  path_.erase(path_.begin());
  checkPath(path_);
  uriPrefix_ += "/";
}

std::string t_package::get_uri(const std::string& name) const {
  if (empty()) {
    return {}; // Package is empty, so no URI.
  }
  return uriPrefix_ + name;
}

t_package::t_package(
    std::vector<std::string> domain, std::vector<std::string> path)
    : domain_(std::move(domain)), path_(std::move(path)) {
  checkDomain(domain_);
  checkPath(path_);
  uriPrefix_ = genPrefix(domain_, path_);
}

} // namespace compiler
} // namespace thrift
} // namespace apache
