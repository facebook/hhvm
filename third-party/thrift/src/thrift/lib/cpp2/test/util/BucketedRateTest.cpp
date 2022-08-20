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

#include <thrift/lib/cpp2/util/BucketedRate.h>

#include <folly/portability/GTest.h>

#include <folly/Random.h>

#include <thrift/lib/cpp2/test/util/FakeClock.h>

namespace apache {
namespace thrift {

FakeClock::time_point FakeClock::now_us_;

class BucketedRateTest : public testing::Test {};

TEST_F(BucketedRateTest, testConstantRate) {
  BucketedRate<FakeClock> rate(std::chrono::seconds(5));
  for (int i = 0; i < 50; i++) {
    rate.tick();
    FakeClock::advance(std::chrono::milliseconds(100));
  }

  ASSERT_NEAR(rate.rate(), 10, 1);
  rate.tick();
  FakeClock::advance(std::chrono::milliseconds(100));
  ASSERT_NEAR(rate.rate(), 10, 1);
}

TEST_F(BucketedRateTest, testRateWithGap) {
  BucketedRate<FakeClock> rate(std::chrono::seconds(5));
  for (int i = 0; i < 50; i++) {
    rate.tick();
    FakeClock::advance(std::chrono::milliseconds(100));
  }

  FakeClock::advance(std::chrono::seconds(2));

  for (int i = 0; i < 9; i++) {
    rate.tick();
    FakeClock::advance(std::chrono::milliseconds(100));
  }

  ASSERT_NEAR(rate.rate(), 6, 1);
}

TEST_F(BucketedRateTest, testRateWithLargeGap) {
  BucketedRate<FakeClock> rate(std::chrono::seconds(5));
  for (int i = 0; i < 10; i++) {
    rate.tick();
    FakeClock::advance(std::chrono::seconds(5));
  }

  FakeClock::advance(std::chrono::seconds(100));

  for (int i = 0; i < 55; i++) {
    rate.tick();
    FakeClock::advance(std::chrono::milliseconds(100));
  }

  ASSERT_NEAR(rate.rate(), 10, 1);
}

} // namespace thrift
} // namespace apache
