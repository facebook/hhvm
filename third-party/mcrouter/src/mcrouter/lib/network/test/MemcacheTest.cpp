/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "mcrouter/lib/network/gen/MemcacheMessages.h"

using namespace facebook::memcache;

TEST(Memcache, staticAssert) {
  static_assert(McGetRequest::typeId == 1, "");
  static_assert(McGetReply::typeId == 2, "");

  static_assert(McSetRequest::typeId == 3, "");
  static_assert(McSetReply::typeId == 4, "");

  static_assert(McDeleteRequest::typeId == 5, "");
  static_assert(McDeleteReply::typeId == 6, "");

  static_assert(McLeaseGetRequest::typeId == 7, "");
  static_assert(McLeaseGetReply::typeId == 8, "");

  static_assert(McLeaseSetRequest::typeId == 9, "");
  static_assert(McLeaseSetReply::typeId == 10, "");

  static_assert(McAddRequest::typeId == 11, "");
  static_assert(McAddReply::typeId == 12, "");

  static_assert(McReplaceRequest::typeId == 13, "");
  static_assert(McReplaceReply::typeId == 14, "");

  static_assert(McGetsRequest::typeId == 15, "");
  static_assert(McGetsReply::typeId == 16, "");

  static_assert(McCasRequest::typeId == 17, "");
  static_assert(McCasReply::typeId == 18, "");

  static_assert(McIncrRequest::typeId == 19, "");
  static_assert(McIncrReply::typeId == 20, "");

  static_assert(McDecrRequest::typeId == 21, "");
  static_assert(McDecrReply::typeId == 22, "");

  static_assert(McMetagetRequest::typeId == 23, "");
  static_assert(McMetagetReply::typeId == 24, "");

  static_assert(McVersionRequest::typeId == 25, "");
  static_assert(McVersionReply::typeId == 26, "");

  static_assert(McAppendRequest::typeId == 27, "");
  static_assert(McAppendReply::typeId == 28, "");

  static_assert(McPrependRequest::typeId == 29, "");
  static_assert(McPrependReply::typeId == 30, "");

  static_assert(McTouchRequest::typeId == 31, "");
  static_assert(McTouchReply::typeId == 32, "");

  static_assert(McStatsRequest::typeId == 33, "");
  static_assert(McStatsReply::typeId == 34, "");

  static_assert(McShutdownRequest::typeId == 35, "");
  static_assert(McShutdownReply::typeId == 36, "");

  static_assert(McQuitRequest::typeId == 37, "");
  static_assert(McQuitReply::typeId == 38, "");

  static_assert(McExecRequest::typeId == 39, "");
  static_assert(McExecReply::typeId == 40, "");

  static_assert(McFlushReRequest::typeId == 41, "");
  static_assert(McFlushReReply::typeId == 42, "");

  static_assert(McFlushAllRequest::typeId == 43, "");
  static_assert(McFlushAllReply::typeId == 44, "");

  static_assert(McGatRequest::typeId == 45, "");
  static_assert(McGatReply::typeId == 46, "");

  static_assert(McGatsRequest::typeId == 47, "");
  static_assert(McGatsReply::typeId == 48, "");
}
