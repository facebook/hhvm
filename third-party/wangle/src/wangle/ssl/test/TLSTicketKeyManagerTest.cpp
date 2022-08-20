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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <wangle/ssl/SSLStats.h>
#include <wangle/ssl/TLSTicketKeyManager.h>
#include <wangle/ssl/test/MockSSLStats.h>

using ::testing::InSequence;
using wangle::MockSSLStats;

TEST(TLSTicketKeyManager, TestSetGetTLSTicketKeySeeds) {
  std::vector<std::string> origOld = {"67"};
  std::vector<std::string> origCurr = {"68"};
  std::vector<std::string> origNext = {"69"};

  wangle::TLSTicketKeyManager manager;

  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
  std::vector<std::string> old;
  std::vector<std::string> curr;
  std::vector<std::string> next;
  manager.getTLSTicketKeySeeds(old, curr, next);
  ASSERT_EQ(origOld, old);
  ASSERT_EQ(origCurr, curr);
  ASSERT_EQ(origNext, next);
}

TEST(TLSTicketKeyManager, TestValidateTicketSeedsSuccess) {
  MockSSLStats stats;
  EXPECT_CALL(stats, recordTLSTicketRotation(true)).Times(2);

  std::vector<std::string> origOld = {"67", "77"};
  std::vector<std::string> origCurr = {"68", "78"};
  std::vector<std::string> origNext = {"69", "79"};

  // The new ticket seeds are compatible
  std::vector<std::string> newOld = {"68", "78"};
  std::vector<std::string> newCurr = {"69", "79"};
  std::vector<std::string> newNext = {"70", "80"};

  wangle::TLSTicketKeyManager manager;
  manager.setStats(&stats);
  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
  manager.setTLSTicketKeySeeds(newOld, newCurr, newNext);
}

TEST(TLSTicketKeyManager, TestValidateTicketSeedsIdempotent) {
  MockSSLStats stats;
  EXPECT_CALL(stats, recordTLSTicketRotation(true)).Times(2);

  std::vector<std::string> origOld = {"67", "77"};
  std::vector<std::string> origCurr = {"68", "78"};
  std::vector<std::string> origNext = {"69", "79"};

  wangle::TLSTicketKeyManager manager;
  manager.setStats(&stats);
  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
}

TEST(TLSTicketKeyManager, TestValidateTicketSeedsFailure) {
  MockSSLStats stats;
  InSequence inSequence;
  EXPECT_CALL(stats, recordTLSTicketRotation(true)).Times(1);
  EXPECT_CALL(stats, recordTLSTicketRotation(false)).Times(1);

  std::vector<std::string> origOld = {"67", "77"};
  std::vector<std::string> origCurr = {"68", "78"};
  std::vector<std::string> origNext = {"69", "79"};

  // The new seeds are incompatible
  std::vector<std::string> newOld = {"69", "79"};
  std::vector<std::string> newCurr = {"70", "80"};
  std::vector<std::string> newNext = {"71", "81"};

  wangle::TLSTicketKeyManager manager;
  manager.setStats(&stats);
  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
  manager.setTLSTicketKeySeeds(newOld, newCurr, newNext);
}

TEST(TLSTicketKeyManager, TestValidateTicketSeedsSubsetPass) {
  MockSSLStats stats;
  InSequence inSequence;
  EXPECT_CALL(stats, recordTLSTicketRotation(true)).Times(2);

  std::vector<std::string> origOld = {"67"};
  std::vector<std::string> origCurr = {"68"};
  std::vector<std::string> origNext = {"69"};

  // The new ticket seeds are compatible
  std::vector<std::string> newOld = {"68", "78"};
  std::vector<std::string> newCurr = {"69"};
  std::vector<std::string> newNext = {"70", "80"};

  wangle::TLSTicketKeyManager manager;
  manager.setStats(&stats);
  manager.setTLSTicketKeySeeds(origOld, origCurr, origNext);
  manager.setTLSTicketKeySeeds(newOld, newCurr, newNext);
}
