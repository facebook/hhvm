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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/server/ReactiveToggle.h>

using namespace apache::thrift::server;

TEST(ReactiveToggleTest, SingleReactiveToggle) {
  ReactiveToggleSource source;

  ReactiveToggle toggle(source);

  EXPECT_EQ(toggle.get(), true);

  int counter = 0;

  folly::Function<void(bool)> func = [&counter](bool on) {
    if (on) {
      ++counter;
    } else {
      --counter;
    }
  };

  auto handle = toggle.addCallback(std::move(func));
  EXPECT_EQ(counter, 1);

  source.set(false);
  EXPECT_EQ(toggle.get(), false);
  EXPECT_EQ(counter, 0);

  source.set(false);
  EXPECT_EQ(toggle.get(), false);
  EXPECT_EQ(counter, 0);

  source.set(true);
  EXPECT_EQ(toggle.get(), true);
  EXPECT_EQ(counter, 1);

  handle.cancelAndJoin();

  source.set(false);
  EXPECT_EQ(toggle.get(), false);
  EXPECT_EQ(counter, 1);

  source.set(true);
  EXPECT_EQ(counter, 1);

  int counter2 = 0;

  folly::Function<void(bool)> func2 = [&counter2](bool) { ++counter2; };

  auto handle2 = toggle.addCallback(std::move(func2));
  EXPECT_EQ(counter2, 1);

  for (int i = 0; i < 100000; ++i) {
    if (i % 2 == 0) {
      source.set(false);
    } else {
      source.set(true);
    }
  }

  EXPECT_EQ(counter2, 100001);
}

TEST(ReactiveToggleTest, MultipleReactiveToggles) {
  ReactiveToggleSource source;

  int counter = 0;

  std::vector<ReactiveToggle> toggles;
  toggles.reserve(9999);

  // in this case the handles are not preserved
  for (int i = 0; i < 9999; ++i) {
    toggles.emplace_back(source);
  }

  for (auto& spot : toggles) {
    folly::Function<void(bool)> func = [&counter](bool) { ++counter; };
    spot.addCallback(std::move(func));
  }

  EXPECT_EQ(counter, 9999);

  // because handles are not preserved
  // callbacks are destroyed
  // so this should be a no-op
  source.set(false);
  EXPECT_EQ(counter, 9999);

  toggles.clear();

  // this time we preserve the handles
  for (int i = 0; i < 9999; ++i) {
    toggles.emplace_back(source);
  }

  std::vector<ReactiveToggle::CallbackHandle> handles;
  handles.reserve(9999);

  for (auto& spot : toggles) {
    folly::Function<void(bool)> func = [&counter](bool) { ++counter; };
    handles.emplace_back(spot.addCallback(std::move(func)));
  }

  EXPECT_EQ(counter, 9999 * 2);

  source.set(true);
  EXPECT_EQ(counter, 9999 * 3);
}

TEST(ReactiveToggle, CancelInTheMiddle) {
  ReactiveToggleSource source;

  int counter = 0;

  ReactiveToggle toggle(source);

  auto handle1 = toggle.addCallback([&counter](bool) { ++counter; });
  auto handle2 = toggle.addCallback([&counter](bool) { ++counter; });
  auto handle3 = toggle.addCallback([&counter](bool) { ++counter; });
  auto handle4 = toggle.addCallback([&counter](bool) { ++counter; });

  EXPECT_EQ(counter, 4);

  handle2.cancelAndJoin();
  handle3.cancelAndJoin();

  source.set(false);

  EXPECT_EQ(counter, 6);

  handle1.cancelAndJoin();
  handle4.cancelAndJoin();

  source.set(true);

  EXPECT_EQ(counter, 6);
}
