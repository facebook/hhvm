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

#include <thrift/common/universal_name.h>

#include <stdexcept>
#include <boost/container/small_vector.hpp>

namespace apache::thrift {
namespace {

void check(bool cond, const char* err) {
  if (!cond) {
    throw std::invalid_argument(err);
  }
}

bool is_domain_char(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || c == '-';
}

bool is_path_char(char c) {
  return is_domain_char(c) || c == '_';
}

bool is_type_char(char c) {
  return is_path_char(c) || (c >= 'A' && c <= 'Z');
}

void check_domain_label(std::string_view label) {
  check(!label.empty(), "empty domain label");
  for (char c : label) {
    check(is_domain_char(c), "invalid domain char");
  }
}

void check_path_segment(std::string_view seg) {
  check(!seg.empty(), "empty path segment");
  for (char c : seg) {
    check(is_path_char(c), "invalid path char");
  }
}

void check_type_segment(std::string_view seg) {
  check(!seg.empty(), "empty type segment");
  for (char c : seg) {
    check(is_type_char(c), "invalid type char");
  }
}

void split(
    boost::container::small_vector<std::string_view, 4>& result,
    std::string_view input,
    char delimiter) {
  size_t start = 0, size = input.size();
  while (start <= size) {
    size_t end = input.find(delimiter, start);
    if (end == std::string_view::npos) {
      result.emplace_back(input.substr(start));
      break;
    }
    result.emplace_back(input.substr(start, end - start));
    start = end + 1;
  }
}

} // namespace

namespace detail {

void check_univeral_name_domain(const std::vector<std::string>& domain) {
  check(domain.size() >= 2, "not enough domain labels");
  for (const std::string& label : domain) {
    check_domain_label(label);
  }
}

void check_universal_name_path(const std::vector<std::string>& path) {
  check(!path.empty(), "not enough path segments");
  for (const std::string& seg : path) {
    check_path_segment(seg);
  }
}

} // namespace detail

void validate_universal_name(std::string_view uri) {
  // We require a minimum 1 domain and 2 path segements, though up to 4 path
  // segments is likely to be common.
  boost::container::small_vector<std::string_view, 4> path;
  split(path, uri, '/');
  check(path.size() >= 3, "not enough path segments");

  // We require a minimum of 2 domain segments, but up to 4 is likely to be
  // common.
  boost::container::small_vector<std::string_view, 4> domain;
  split(domain, path[0], '.');
  check(domain.size() >= 2, "not enough domain labels");
  for (std::string_view label : domain) {
    check_domain_label(label);
  }

  size_t i = 1;
  for (; i < path.size() - 1; ++i) {
    check_path_segment(path[i]);
  }
  check_type_segment(path[i]);
}

} // namespace apache::thrift
