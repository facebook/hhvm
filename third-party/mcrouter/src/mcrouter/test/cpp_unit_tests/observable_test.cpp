/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <atomic>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include "mcrouter/Observable.h"

using facebook::memcache::mcrouter::CallbackPool;
using facebook::memcache::mcrouter::Observable;

struct NoCopy {
  int x;

  explicit NoCopy(int x_ = 0) : x(x_) {}
  NoCopy(const NoCopy&) = delete;
  NoCopy(NoCopy&&) = default;

  NoCopy& operator=(const NoCopy&) = delete;
  NoCopy& operator=(NoCopy&&) = default;
};

struct NoCopyNoMove {
  NoCopyNoMove() = default;
  NoCopyNoMove(const NoCopyNoMove&) = delete;
  NoCopyNoMove(NoCopyNoMove&&) = delete;

  NoCopyNoMove& operator=(const NoCopyNoMove&) = delete;
  NoCopyNoMove& operator=(NoCopyNoMove&&) = delete;
};

TEST(CallbackPool, sanity) {
  CallbackPool<const NoCopyNoMove&> pool;
  int step = 0;

  auto onUpdate = [&step](const NoCopyNoMove&) { ++step; };

  auto handle1 = pool.subscribe(onUpdate);
  auto handle2 = pool.subscribe(onUpdate);
  pool.notify(NoCopyNoMove());

  EXPECT_EQ(step, 2);

  handle2.reset();
  pool.notify(NoCopyNoMove());

  EXPECT_EQ(step, 3);
}

TEST(Observable, sanity) {
  Observable<NoCopy> data(NoCopy(-1));

  int step = 0;

  int prev = 0;
  int cur = -1;

  auto onUpdate = [&step, &prev, &cur](
                      const NoCopy& oldData, const NoCopy& newData) {
    EXPECT_EQ(oldData.x, prev);
    EXPECT_EQ(newData.x, cur);

    ++step;
  };

  auto handle1 = data.subscribe(onUpdate);
  EXPECT_EQ(step, 0);
  auto handle2 = data.subscribeAndCall(onUpdate);
  EXPECT_EQ(step, 1);

  for (int i = 0; i < 3; ++i) {
    prev = cur;
    cur = i;
    data.emplace(cur);
    EXPECT_EQ(step, (i + 1) * 2 + 1);
  }
}

TEST(Observable, rand) {
  Observable<int> data(1);

  const int kValueToAchieve = 1 << 8;
  const int kSleepRange = 10;

  auto consumerFunc = [&data]() {
    std::atomic<bool> finished(false);

    auto onUpdate = [&finished](int oldData, int newData) {
      EXPECT_EQ(oldData + 1, newData);
      if (newData == kValueToAchieve) {
        finished = true;
      }
    };

    auto handle = data.subscribe(onUpdate);

    while (!finished) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds((rand() % kSleepRange) + 1));
    }
  };

  auto producerFunc = [&data]() {
    while (true) {
      auto val = data.get();
      if (val == kValueToAchieve) {
        return;
      }

      data.set(val + 1);

      std::this_thread::sleep_for(
          std::chrono::milliseconds((rand() % kSleepRange) + 1));
    }
  };

  std::unique_ptr<std::thread> consumer[16];
  std::unique_ptr<std::thread> producer;

  for (auto i = 0; i < 16; ++i) {
    consumer[i] = std::make_unique<std::thread>(consumerFunc);
  }

  producer = std::make_unique<std::thread>(producerFunc);

  producer->join();
  for (auto i = 0; i < 16; ++i) {
    consumer[i]->join();
  }
}
