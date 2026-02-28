/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>

#include <gtest/gtest.h>

#include "mcrouter/route.h"

using facebook::memcache::mcrouter::match_pattern_route;
using std::string;

void expect_match(const string& p, const string& r) {
  EXPECT_TRUE(match_pattern_route(p, r));
}

void expect_no_match(const string& p, const string& r) {
  EXPECT_FALSE(match_pattern_route(p, r));
}

TEST(MatchPatternRoute, sanity) {
  expect_match("/a/b/", "/a/b/");
  expect_match("/a/*/", "/a/b/");
  expect_match("/a/a*c/", "/a/abc/");
  expect_match("/a/a*c/", "/a/abbbbbbbbbbbbbbbc/");
  expect_match("/a*c/d*f/", "/abbbbc/deeeeeeeeeeeef/");
  expect_match("/a****/d*f/", "/abbbbc/deeeeeeeeeeeef/");
  expect_match("/*/*/", "/aaa/bbb/");
  expect_match("/*baf*/", "/aaabafggg/");
  expect_match("/*1*2*3*4*5/", "/aaa1bbb2bbb3af4sdgfsdg5/");
  expect_match("*", "a");
  expect_match("/*a/a/", "/a/a/");
  expect_match("/a/*a/", "/a/a/");

  expect_no_match("*", "");
  expect_no_match("*", "/");
  expect_no_match("*/*/", "a/b");
  expect_no_match("*", "a/b");
  expect_no_match("****", "/b");
}
