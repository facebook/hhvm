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

#include <cctype>
#include <stdexcept>
#include <boost/container/small_vector.hpp>
#include <fmt/core.h>
#include <fmt/format.h>

namespace apache::thrift {
/**
 * Trivial wrapper type around a `char`, to allow safe formatting of
 * non-printable characters (as their hexadecimal literal representation, eg.
 * '\x1c').
 */
struct printable_char final {
  explicit printable_char(char c) : c(c) {}

  char c;
};
} // namespace apache::thrift

/**
 * Formats a `printable_char` with the corresponding (printable or hexadecimal)
 * character literal.
 */
template <>
struct fmt::formatter<apache::thrift::printable_char>
    : fmt::formatter<std::string> {
  format_context::iterator format(
      apache::thrift::printable_char p, format_context& ctx) const {
    std::string s = std::isprint(p.c) ? fmt::format("'{}'", p.c)
                                      : fmt::format(R"('\x{:x}')", p.c);
    return formatter<std::string>::format(s, ctx);
  }
};

namespace apache::thrift {
namespace {

/**
 * Throws an `invalid_argument` exception with the given (formatted) message if
 * `cond` is `false`.
 */
template <typename... T>
void check(bool cond, fmt::format_string<T...> msg, T&&... args) {
  if (!cond) [[unlikely]] {
    throw std::invalid_argument(fmt::format(msg, std::forward<T>(args)...));
  }
}

bool is_domain_char(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z');
}

bool is_path_char(char c) {
  return is_domain_char(c) || c == '_';
}

bool is_type_char(char c) {
  return is_path_char(c) || (c >= 'A' && c <= 'Z');
}

template <typename TStringContainer>
void check_domain_components(const TStringContainer& domain) {
  check(
      domain.size() >= 2,
      "Not enough domain components: expected at least 2, got {}",
      domain.size());
  for (std::size_t i = 0; i < domain.size(); ++i) {
    const auto& component = domain[i];
    check(!component.empty(), "URI domain component at index {} is empty", i);

    for (std::size_t j = 0; j < component.size(); ++j) {
      char c = component[j];
      check(
          is_domain_char(c),
          "URI domain component #{} has invalid character at position {}: {}",
          i,
          j,
          printable_char(c));
    }
  }
}

template <typename TStringishIterator>
void check_path_segments(TStringishIterator begin, TStringishIterator end) {
  check(begin != end, "Empty URI path");

  std::size_t i = 0;
  for (TStringishIterator it = begin; it != end; ++it, ++i) {
    const auto& segment = *it;
    check(!segment.empty(), "URI path segment at index {} is empty", i);

    for (std::size_t j = 0; j < segment.size(); ++j) {
      char c = segment[j];
      check(
          is_path_char(c),
          "URI path segment #{} has invalid character at position {}: {}",
          i,
          j,
          printable_char(c));
    }
  }
}

void check_type_segment(std::string_view segment) {
  check(!segment.empty(), "Empty URI type segment");
  for (std::size_t j = 0; j < segment.size(); ++j) {
    char c = segment[j];
    check(
        is_type_char(c),
        "URI type segment has invalid character at position {}: {}",
        j,
        printable_char(c));
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
  try {
    check_domain_components(domain);
  } catch (const std::invalid_argument& ex) {
    throw std::invalid_argument(
        fmt::format("Invalid Thrift URI domain ({}).", ex.what()));
  }
}

void check_universal_name_path(const std::vector<std::string>& path) {
  try {
    check_path_segments(path.begin(), path.end());
  } catch (const std::invalid_argument& ex) {
    throw std::invalid_argument(
        fmt::format("Invalid Thrift URI path ({}).", ex.what()));
  }
}

} // namespace detail

void validate_universal_name(std::string_view uri) {
  // A valid Thrift universal name (aka Thrift URI) must have at least 3
  // parts separated by "/":
  // 1. A domain (eg. "facebook.com")
  // 2. A non-empty path
  // 3. A type name
  //
  // e.g.: "facebook.com/thrift/Value"
  //
  // We expect most will have at least 4 parts, so initializing the container
  // accordingly.
  boost::container::small_vector<std::string_view, 4> uri_parts;
  split(uri_parts, uri, '/');
  try {
    check(
        uri_parts.size() >= 3,
        "Not enough parts: expected at least 3, got: {}",
        uri_parts.size());

    // We require a minimum of 2 domain segments, but up to 4 is likely to be
    // common.
    boost::container::small_vector<std::string_view, 4> domain;
    split(domain, uri_parts[0], '.');
    check_domain_components(domain);
    check_path_segments(uri_parts.begin() + 1, uri_parts.end() - 1);
    check_type_segment(uri_parts.back());
  } catch (const std::invalid_argument& ex) {
    throw std::invalid_argument(
        fmt::format("Not a valid Thrift URI: \"{}\" ({})", uri, ex.what()));
  }
}

} // namespace apache::thrift
