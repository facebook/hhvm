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

#include <thrift/lib/cpp2/util/EwmaRate.h>

#include <gtest/gtest.h>

#include <folly/Random.h>

#include <thrift/lib/cpp2/test/util/FakeClock.h>

namespace apache::thrift {

FakeClock::time_point FakeClock::now_us_;

class EwmaRateTest : public testing::Test {};

TEST_F(EwmaRateTest, testConstantRate) {
  EwmaRate<FakeClock> rate(std::chrono::seconds(1));
  for (int i = 0; i < 100; i++) {
    rate.tick();
    FakeClock::advance(std::chrono::seconds(1));
  }

  ASSERT_NEAR(rate.rate(), 1, 0.1);
}

TEST_F(EwmaRateTest, testIncreasingRate) {
  EwmaRate<FakeClock> rate(std::chrono::seconds(1));
  for (int i = 0; i < 10; i++) {
    rate.tick();
    FakeClock::advance(std::chrono::seconds(2));
  }

  for (int i = 0; i < 200; i++) {
    rate.tick();
    FakeClock::advance(std::chrono::milliseconds(100));
  }

  ASSERT_NEAR(rate.rate(), 10, 0.1);
}

TEST_F(EwmaRateTest, testNonConstantRate) {
  EwmaRate<FakeClock> rate(std::chrono::seconds(1));
  for (int i = 0; i < 1000; i++) {
    rate.tick();
    auto dt = 450 + folly::Random::rand32(100); // dt in [450, 550] avg at 500ms
    FakeClock::advance(std::chrono::milliseconds(dt));
  }

  ASSERT_NEAR(rate.rate(), 2, 0.2);
}

} // namespace apache::thrift
