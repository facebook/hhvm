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

#include <thrift/lib/cpp2/util/Ewma.h>

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/test/util/FakeClock.h>

namespace apache::thrift {

FakeClock::time_point FakeClock::now_us_;

class EwmaTest : public testing::Test {};

TEST_F(EwmaTest, testInitialValue) {
  Ewma<FakeClock> ewma(std::chrono::seconds(5), 0);
  ASSERT_EQ(ewma.estimate(), 0.0);

  Ewma<FakeClock> ewma2(std::chrono::seconds(5), 50);
  ASSERT_EQ(ewma2.estimate(), 50.0);
}

TEST_F(EwmaTest, testConstantValue) {
  Ewma<FakeClock> ewma(std::chrono::seconds(5), 0);

  auto target = 100.0;
  for (int i = 0; i < 100; i++) {
    ewma.add(target);
    FakeClock::advance(std::chrono::seconds(1));
  }
  ASSERT_NEAR(ewma.estimate(), target, target * 0.1 / 100);
}

TEST_F(EwmaTest, testChangeInConstantValue) {
  Ewma<FakeClock> ewma(std::chrono::seconds(5), 0);

  auto target = 100.0;
  for (int i = 0; i < 100; i++) {
    ewma.add(target);
    FakeClock::advance(std::chrono::seconds(1));
  }

  auto target2 = 1000.0;
  for (int i = 0; i < 100; i++) {
    ewma.add(target2);
    FakeClock::advance(std::chrono::seconds(1));
  }

  // Should converge to the second target
  ASSERT_NEAR(ewma.estimate(), target2, target2 * 0.1 / 100);
}

TEST_F(EwmaTest, testHalfLife) {
  auto halfLife = std::chrono::seconds(5);
  auto initial = 100;
  Ewma<FakeClock> ewma(halfLife, initial);

  auto previous = ewma.estimate();
  auto target2 = 1000.0;
  FakeClock::advance(halfLife);
  ewma.add(target2);

  auto expectedValue =
      previous + (target2 - previous) / 2; // half of the difference
  ASSERT_NEAR(ewma.estimate(), expectedValue, expectedValue * 0.1 / 100);
}

} // namespace apache::thrift
