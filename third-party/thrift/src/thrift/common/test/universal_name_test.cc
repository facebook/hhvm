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

#include <regex>
#include <fmt/core.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace apache::thrift {
namespace {

using testing::ThrowsMessage;

const auto good_def_names = {
    "type",
    "Type",
    "other_type",
    "4",
};

const auto bad_def_names = {
    "ty%20pe",
    "/type",
    "type?",
    "type?a=",
    "type?a=b",
    "type#",
    "type#1",
    "type@",
    "type@1",
    "other-type",
};

const auto good_package_names = {
    "foo.com/my",
    "foo.com/m_y",
    "foo.com/my/type",
    "1.2/3",
};

const auto bad_package_names = {
    "my",
    "foo/my",
    "Foo.com/my",
    "foo.Com/my",
    "foo.com/My",
    "foo.com:42/my",
    "foo%20.com/my",
    "foo.com/m%20y",
    "@foo.com/my",
    ":@foo.com/my",
    ":foo.com/my",
    "user@foo.com/my",
    "user:pass@foo.com/my",
    "fbthrift://foo.com/my",
    ".com/my",
    "foo./my",
    "./my",
    "/my",
    "foo.com/m#y",
    "foo.com/m@y",
    "foo.com/my/ty@pe",
    "foo_bar.com/my",
    "foo-bar.com/my",
    "foo.com/m-y",
};

std::string gen_uri(const char* package, const char* name) {
  return fmt::format("{}/{}", package, name);
}

std::vector<std::string> gen_good_def_uris() {
  std::vector<std::string> result;
  for (const char* package : good_package_names) {
    for (const char* name : good_def_names) {
      result.emplace_back(gen_uri(package, name));
    }
  }
  return result;
}

std::vector<std::string> gen_bad_def_uris() {
  std::vector<std::string> result;
  for (const char* package : good_package_names) {
    for (const char* name : bad_def_names) {
      result.emplace_back(gen_uri(package, name));
    }
  }
  for (const char* package : bad_package_names) {
    for (const char* name : good_def_names) {
      result.emplace_back(gen_uri(package, name));
    }
  }
  for (const char* package : bad_package_names) {
    for (const char* name : bad_def_names) {
      result.emplace_back(gen_uri(package, name));
    }
  }
  return result;
}

void expect_validate_error(
    std::string_view uri, std::string_view expected_error_message) {
  SCOPED_TRACE(uri);
  EXPECT_THAT(
      [&]() { validate_universal_name(uri); },
      ThrowsMessage<std::invalid_argument>(expected_error_message));
}

} // namespace

TEST(UniversalNameTest, validate_universal_name_bulk) {
  // @lint-ignore-every CLANGTIDY facebook-hte-StdRegexIsAwful
  std::regex pattern(fmt::format(
      "{0}(\\.{0})+\\/{1}(\\/{1})*(\\/{2})",
      "[a-z0-9]+",
      "[a-z0-9_]+",
      "[a-zA-Z0-9_]+"));
  for (const std::string& good : gen_good_def_uris()) {
    SCOPED_TRACE(good);
    EXPECT_TRUE(std::regex_match(good, pattern));
    validate_universal_name(good);
  }

  for (const std::string& bad : gen_bad_def_uris()) {
    SCOPED_TRACE(bad);
    EXPECT_FALSE(std::regex_match(bad, pattern));
    EXPECT_THROW(validate_universal_name(bad), std::invalid_argument);
  }
}

TEST(UniversalNameTest, validate_universal_name_canonical) {
  // GOOD: Canonical
  validate_universal_name("facebook.com/thrift/Value");
}

TEST(UniversalNameTest, validate_universal_name_missing_path_type) {
  // BAD: Missing path, type
  expect_validate_error(
      "facebook.com",
      R"(Not a valid Thrift URI: "facebook.com" (Not enough parts: expected at least 3, got: 1))");
}

TEST(UniversalNameTest, validate_universal_name_missing_type) {
  // BAD: Missing type
  expect_validate_error(
      "facebook.com/thrift",
      R"(Not a valid Thrift URI: "facebook.com/thrift" (Not enough parts: expected at least 3, got: 2))");
}

TEST(
    UniversalNameTest,
    validate_universal_name_invalid_printable_character_type) {
  // BAD: Invalid (printable) character in type
  expect_validate_error(
      "facebook.com/thrift/Value!",
      R"(Not a valid Thrift URI: "facebook.com/thrift/Value!" (URI type segment has invalid character at position 5: '!'))");
}

TEST(
    UniversalNameTest,
    validate_universal_name_invalid_nonprintable_character_type) {
  // BAD: Invalid (non-printable) character in type
  expect_validate_error(
      "facebook.com/thrift/Val\x1cue",
      R"(Not a valid Thrift URI: "facebook.com/thrift/Val)"
      "\x1c"
      R"(ue" (URI type segment has invalid character at position 3: '\x1c'))");
}

TEST(UniversalNameTest, validate_universal_name_invalid_uppercase_domain) {
  // BAD: Uppercase in domain
  expect_validate_error(
      "Facebook.com/thrift/Value",
      R"(Not a valid Thrift URI: "Facebook.com/thrift/Value" (URI domain component #0 has invalid character at position 0: 'F'))");
}

TEST(UniversalNameTest, validate_universal_name_invalid_uppercase_path) {
  // BAD: Uppercase in path
  expect_validate_error(
      "facebook.com/Thrift/Value",
      R"(Not a valid Thrift URI: "facebook.com/Thrift/Value" (URI path segment #0 has invalid character at position 0: 'T'))");
  expect_validate_error(
      "facebook.com/thrift/paTh/Value",
      R"(Not a valid Thrift URI: "facebook.com/thrift/paTh/Value" (URI path segment #1 has invalid character at position 2: 'T'))");
}

TEST(UniversalNameTest, validate_universal_name_invalid_empty_path_segment) {
  // BAD: Has an empty path segment
  expect_validate_error(
      "facebook.com//thrift/Value",
      R"(Not a valid Thrift URI: "facebook.com//thrift/Value" (URI path segment at index 0 is empty))");

  expect_validate_error(
      "facebook.com/thrift//Value",
      R"(Not a valid Thrift URI: "facebook.com/thrift//Value" (URI path segment at index 1 is empty))");
}

TEST(
    UniversalNameTest, validate_universal_name_invalid_empty_domain_component) {
  // BAD: Has an empty domain component
  expect_validate_error(
      ".facebook.com./thrift/Value",
      R"(Not a valid Thrift URI: ".facebook.com./thrift/Value" (URI domain component at index 0 is empty))");

  expect_validate_error(
      "facebook..com./thrift/Value",
      R"(Not a valid Thrift URI: "facebook..com./thrift/Value" (URI domain component at index 1 is empty))");

  expect_validate_error(
      "facebook.com./thrift/Value",
      R"(Not a valid Thrift URI: "facebook.com./thrift/Value" (URI domain component at index 2 is empty))");
}

} // namespace apache::thrift
