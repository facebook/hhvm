/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <condition_variable>
#include <mutex>
#include <string>

#include <gtest/gtest.h>

#include <folly/FileUtil.h>
#include <folly/experimental/TestUtil.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include "mcrouter/FileObserver.h"

using facebook::memcache::mcrouter::startObservingFile;
using folly::test::TemporaryFile;

const std::string BOGUS_CONFIG = "this/file/doesnot/exists";

TEST(FileObserver, sanity) {
  folly::test::TemporaryFile config("file_observer_test");
  std::string path(config.path().string());
  std::string contents = "a";
  std::mutex mut;
  std::condition_variable cv;

  EXPECT_EQ(
      folly::writeFull(config.fd(), contents.data(), contents.size()),
      contents.size());

  const auto scheduler = std::make_shared<folly::FunctionScheduler>();
  scheduler->start();
  int counter = 0;
  auto handle = startObservingFile(
      path,
      scheduler,
      std::chrono::milliseconds(100),
      std::chrono::milliseconds(500),
      [&counter, &mut, &cv](std::string) {
        std::lock_guard<std::mutex> lock(mut);
        counter++;
        cv.notify_all();
      });
  EXPECT_TRUE(handle);

  EXPECT_EQ(counter, 1);
  contents = "b";
  EXPECT_EQ(
      folly::writeFull(config.fd(), contents.data(), contents.size()),
      contents.size());
  {
    std::unique_lock<std::mutex> lock(mut);
    cv.wait_for(lock, std::chrono::seconds(5));
  }
  EXPECT_EQ(counter, 2);
  contents = "c";
  EXPECT_EQ(
      folly::writeFull(config.fd(), contents.data(), contents.size()),
      contents.size());
  {
    std::unique_lock<std::mutex> lock(mut);
    cv.wait_for(lock, std::chrono::seconds(5));
  }
  EXPECT_EQ(counter, 3);

  // Stop observing, then touch the file and verify the counter is not changed.
  handle.reset();
  contents = "d";
  EXPECT_EQ(
      folly::writeFull(config.fd(), contents.data(), contents.size()),
      contents.size());
  {
    std::unique_lock<std::mutex> lock(mut);
    cv.wait_for(lock, std::chrono::seconds(1));
  }
  EXPECT_EQ(counter, 3);
}

TEST(FileObserver, on_error_callback) {
  const auto scheduler = std::make_shared<folly::FunctionScheduler>();
  scheduler->start();
  int successCounter1 = 0;
  auto handle1 = startObservingFile(
      BOGUS_CONFIG,
      scheduler,
      std::chrono::milliseconds(100),
      std::chrono::milliseconds(500),
      [&successCounter1](std::string) { successCounter1++; });

  int successCounter2 = 0;
  auto handle2 = startObservingFile(
      "",
      scheduler,
      std::chrono::milliseconds(100),
      std::chrono::milliseconds(500),
      [&successCounter2](std::string) { successCounter2++; });

  EXPECT_EQ(successCounter1, 0);
  EXPECT_FALSE(handle1);

  EXPECT_EQ(successCounter2, 0);
  EXPECT_FALSE(handle2);
}
