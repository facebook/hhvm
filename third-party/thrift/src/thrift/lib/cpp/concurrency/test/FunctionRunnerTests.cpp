/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/lib/cpp/concurrency/FunctionRunner.h>

#include <atomic>
#include <cstdio>
#include <thread>

#include <folly/portability/GTest.h>

using namespace apache::thrift::concurrency;

TEST(FunctionRunnerTests, NonPeriodicTest) {
  bool didRunInit = false;
  bool didRunFunc = false;

  auto f = FunctionRunner::create([&]() mutable {
    fprintf(stderr, "a");
    fflush(stderr);
    didRunFunc = true;
  });

  f->setInitFunc([&]() mutable {
    fprintf(stderr, "b");
    fflush(stderr);
    didRunInit = true;
  });

  f->run();

  EXPECT_TRUE(didRunInit);
  EXPECT_TRUE(didRunFunc);
}

TEST(FunctionRunnerTests, PeriodicTest) {
  static unsigned kMsec = 25;
  static unsigned kCount = 10;

  size_t counter = 0;

  auto f = FunctionRunner::create(
      [&]() mutable {
        fprintf(stderr, "c");
        fflush(stderr);
        return (++counter) < kCount;
      },
      kMsec);

  f->run();

  EXPECT_EQ(kCount, counter);
}

TEST(FunctionRunnerTests, PeriodicTestWithCancellation) {
  static unsigned kMsec = 25;
  static unsigned kCount = 5;

  std::atomic<size_t> counter(0);
  // this callback always returns true so it runs indefinitely
  auto f = FunctionRunner::create(
      [&]() mutable {
        fprintf(stderr, "d");
        fflush(stderr);
        ++counter;
        return true;
      },
      kMsec);

  // kick off in another thread
  auto t = std::thread([&] { f->run(); });

  // hang out a little while, let it run a few times
  while (kCount > counter) {
    std::this_thread::yield();
  }

  // reset shared_ptr; kills the FunctionRunner
  f.reset();

  // wait for thread to stop (that is, when `run` returns)
  t.join();

  SUCCEED();
}
