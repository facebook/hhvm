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

#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>
#include <wangle/ssl/TLSInMemoryTicketProcessor.h>

using namespace folly;
using namespace wangle;

class TestInMemoryTicket : public testing::Test {};

TEST_F(TestInMemoryTicket, TestInitInMemoryTicket) {
  auto processor = TLSInMemoryTicketProcessor();
  auto seeds = processor.initInMemoryTicketSeeds();
  // When new in memory ticket seeds are created, the old seeds should be empty
  ASSERT_EQ(0, seeds.oldSeeds.size());
  ASSERT_EQ(1, seeds.currentSeeds.size());
  ASSERT_EQ(1, seeds.newSeeds.size());
}

TEST_F(TestInMemoryTicket, TestUpdateInMemoryTicket) {
  Baton<> ticketBaton;
  TLSTicketKeySeeds rotatedSeeds;
  auto callback = [&rotatedSeeds, &ticketBaton](TLSTicketKeySeeds seeds) {
    ticketBaton.post();
    rotatedSeeds = seeds;
  };
  auto processor = TLSInMemoryTicketProcessor(
      {callback},
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::seconds(3)));
  auto originalSeeds = processor.initInMemoryTicketSeeds();
  // Test the processor generates valid ticket seeds
  EXPECT_TRUE(ticketBaton.try_wait_for(std::chrono::seconds(4)));
  EXPECT_TRUE(originalSeeds.isValidRotation(rotatedSeeds));
  ticketBaton.reset();
  originalSeeds = rotatedSeeds;
  // Test the processor still generates valid ticket seeds for second rotation
  EXPECT_TRUE(ticketBaton.try_wait_for(std::chrono::seconds(4)));
  EXPECT_TRUE(originalSeeds.isValidRotation(rotatedSeeds));
}
