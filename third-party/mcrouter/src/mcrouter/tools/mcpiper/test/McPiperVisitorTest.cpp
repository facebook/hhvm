/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>

#include "mcrouter/lib/carbon/test/gen/CarbonTestMessages.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/tools/mcpiper/McPiperVisitor.h"

using carbon::detail::McPiperVisitor;
using namespace facebook::memcache;
using namespace carbon::test;

namespace {

void testBasic(bool scriptMode) {
  McLeaseSetReply msg(carbon::Result::FOUND);
  msg.appSpecificErrorCode_ref() = 17;
  msg.message_ref() = "A message";

  McPiperVisitor v(scriptMode);
  msg.visitFields(v);

  auto str = std::move(v).styled();

  EXPECT_TRUE(str.text().contains("A message"));
  EXPECT_TRUE(str.text().contains("17"));
}

void testComplete(bool scriptMode) {
  TestRequest msg("abc");
  msg.testList_ref() = {"qqq", "www"};
  msg.testUMap_ref() = {{"abc", "def"}, {"a", "b"}};
  msg.testComplexMap_ref() = {
      {"key01", {1, 2, 3}}, {"key02", {5, 6}}, {"key03", {}}};

  McPiperVisitor v(scriptMode);
  msg.visitFields(v);

  auto str = std::move(v).styled();

  EXPECT_TRUE(str.text().contains("qqq"));
  EXPECT_TRUE(str.text().contains("www"));

  EXPECT_TRUE(str.text().contains("abc"));
  EXPECT_TRUE(str.text().contains("def"));

  EXPECT_TRUE(str.text().contains("key01"));
  EXPECT_TRUE(str.text().contains("5"));
}

} // anonymous namespace

TEST(McPiperVisitor, basic) {
  testBasic(false /* scriptMode */);
}
TEST(McPiperVisitor, basic_script) {
  testBasic(true /* scriptMode */);
}

TEST(McPiperVisitor, complete) {
  testComplete(false /* scriptMode */);
}
TEST(McPiperVisitor, complete_script) {
  testComplete(true /* scriptMode */);
}
