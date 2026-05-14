/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "mcrouter/lib/network/MessageHelpers.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

using namespace facebook::memcache;

// Compile-time SFINAE checks: the trait must match exactly the two carbon
// types that carry a non-optional `i64 leaseToken` field, and nothing else.
// If a future Memcache.idl change flips the field type (e.g. to optional or
// terse) the static_asserts will break the build instead of silently dropping
// the column from the mcrouter_requests Scuba dataset.
static_assert(HasLeaseTokenTrait<McLeaseGetReply>::value);
static_assert(HasLeaseTokenTrait<McLeaseSetRequest>::value);
static_assert(!HasLeaseTokenTrait<McLeaseGetRequest>::value);
static_assert(!HasLeaseTokenTrait<McLeaseSetReply>::value);
static_assert(!HasLeaseTokenTrait<McGetRequest>::value);
static_assert(!HasLeaseTokenTrait<McGetReply>::value);
static_assert(!HasLeaseTokenTrait<McSetRequest>::value);
static_assert(!HasLeaseTokenTrait<McSetReply>::value);
static_assert(!HasLeaseTokenTrait<McDeleteRequest>::value);

TEST(MessageHelpersTest, GetLeaseTokenIfExist_LeaseGetReply_ReturnsValue) {
  McLeaseGetReply reply;
  reply.leaseToken() = 12345;
  EXPECT_EQ(getLeaseTokenIfExist(reply), std::optional<int64_t>{12345});
}

TEST(MessageHelpersTest, GetLeaseTokenIfExist_LeaseSetRequest_ReturnsValue) {
  McLeaseSetRequest request("key");
  request.leaseToken() = 67890;
  EXPECT_EQ(getLeaseTokenIfExist(request), std::optional<int64_t>{67890});
}

TEST(MessageHelpersTest, GetLeaseTokenIfExist_LeaseTokenZero_ReturnsZero) {
  // Zero is a legitimate token value and must be distinguishable from absence.
  McLeaseGetReply reply;
  reply.leaseToken() = 0;
  EXPECT_EQ(getLeaseTokenIfExist(reply), std::optional<int64_t>{0});
}

TEST(MessageHelpersTest, GetLeaseTokenIfExist_NonLeaseTypes_ReturnsNullopt) {
  McGetRequest getRequest("key");
  McGetReply getReply;
  McSetRequest setRequest("key");
  McSetReply setReply;
  McLeaseGetRequest leaseGetRequest("key");
  McLeaseSetReply leaseSetReply;

  EXPECT_EQ(getLeaseTokenIfExist(getRequest), std::nullopt);
  EXPECT_EQ(getLeaseTokenIfExist(getReply), std::nullopt);
  EXPECT_EQ(getLeaseTokenIfExist(setRequest), std::nullopt);
  EXPECT_EQ(getLeaseTokenIfExist(setReply), std::nullopt);
  EXPECT_EQ(getLeaseTokenIfExist(leaseGetRequest), std::nullopt);
  EXPECT_EQ(getLeaseTokenIfExist(leaseSetReply), std::nullopt);
}
