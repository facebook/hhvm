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
#include <gtest/gtest.h>

using apache::thrift::validate_universal_name;

namespace {

const auto good_def_names = {
    "type",
    "Type",
    "other-type",
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
};

const auto good_package_names = {
    "foo.com/my",
    "foo.com/m_y",
    "foo.com/m-y",
    "foo-bar.com/my",
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

} // namespace

TEST(UniversalNameTest, validate_universal_name) {
  // @lint-ignore-every CLANGTIDY facebook-hte-StdRegexIsAwful
  std::regex pattern(fmt::format(
      "{0}(\\.{0})+\\/{1}(\\/{1})*(\\/{2})",
      "[a-z0-9-]+",
      "[a-z0-9_-]+",
      "[a-zA-Z0-9_-]+"));
  for (const auto& good : gen_good_def_uris()) {
    SCOPED_TRACE(good);
    EXPECT_TRUE(std::regex_match(good, pattern));
    validate_universal_name(good);
  }

  for (const auto& bad : gen_bad_def_uris()) {
    SCOPED_TRACE(bad);
    EXPECT_FALSE(std::regex_match(bad, pattern));
    EXPECT_THROW(validate_universal_name(bad), std::invalid_argument);
  }
}
