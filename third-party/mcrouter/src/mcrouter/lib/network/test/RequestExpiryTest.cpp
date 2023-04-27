/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sys/uio.h>

#include <chrono>
#include <cstring>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "mcrouter/McReqUtil.h"
#include "mcrouter/lib/network/CaretProtocol.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/test/gen/CarbonTestMessages.h"

using namespace facebook::memcache;

TEST(RequestExpiryTest, BasicTest) {
  facebook::memcache::test::McExpTestRequest req;
  McGetReply reply(carbon::Result::REMOTE_ERROR);

  const std::string message(1024, 'a');
  reply.message_ref() = message;

  req.key_ref() = "abcd";
  req.flags_ref() = 1;
  setRequestDeadline(req, 10);
  EXPECT_FALSE(isRequestDeadlineExceeded(req));
  /* sleep override */
  std::this_thread::sleep_for(std::chrono::milliseconds(11));
  EXPECT_TRUE(isRequestDeadlineExceeded(req));
}
