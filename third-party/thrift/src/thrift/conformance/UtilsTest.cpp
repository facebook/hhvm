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

#include <thrift/conformance/Utils.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace apache::thrift::conformance {
namespace {
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

TEST(UtilsTest, ParseNameAndCmd) {
  EXPECT_THAT(parseNameAndCmd("foo/bar"), Pair("foo", "foo/bar"));
  EXPECT_THAT(parseNameAndCmd("/foo/bar"), Pair("foo", "/foo/bar"));
  EXPECT_THAT(parseNameAndCmd("/baz/foo/bar"), Pair("foo", "/baz/foo/bar"));
  EXPECT_THAT(parseNameAndCmd("/foo/bar/baz"), Pair("bar", "/foo/bar/baz"));
  EXPECT_THAT(parseNameAndCmd("/foo/bar#"), Pair("foo", "/foo/bar"));
  EXPECT_THAT(parseNameAndCmd("/foo/bar##"), Pair("foo", "/foo/bar#"));
  EXPECT_THAT(parseNameAndCmd("/foo/bar#baz"), Pair("baz", "/foo/bar"));
  EXPECT_THAT(parseNameAndCmd("/foo/bar##baz"), Pair("baz", "/foo/bar#"));
  EXPECT_THAT(parseNameAndCmd("/foo/bar#baz#"), Pair("foo", "/foo/bar#baz"));
  EXPECT_THAT(parseNameAndCmd("/foo#bar/baz"), Pair("foo#bar", "/foo#bar/baz"));
}

TEST(UtilsTest, ParseCmds) {
  auto actual = parseCmds("/foo/bar, /foo/baz#biz ,foo#bar/baz");
  EXPECT_THAT(
      actual,
      UnorderedElementsAre(
          Pair("foo", "/foo/bar"),
          Pair("biz", "/foo/baz"),
          Pair("foo#bar", "foo#bar/baz")));
}

TEST(UtilsTest, ParseCmds_Dupe) {
  EXPECT_THROW(parseCmds("/foo/bar,/baz/foo/bar"), std::invalid_argument);
}

TEST(UtilsTest, ParseNonconforming) {
  EXPECT_THAT(
      parseNonconforming(
          " # Comment\n"
          "test1 # Other comment\n"
          " test2"),
      UnorderedElementsAre("test1", "test2"));
}

TEST(UtilsTest, ParseTestSuite) {}

} // namespace
} // namespace apache::thrift::conformance
