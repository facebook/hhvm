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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustMessageAdapter.h>

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace channel_pipeline_rust {
namespace {

using apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

// In-process watchdog: aborts with a diagnostic if the test body does not
// complete within 5 seconds. Matches the convention in PipelineTestHelper.cpp.
class TestWatchdog {
 public:
  explicit TestWatchdog(const char* operation)
      : operation_{operation}, thread_{[this] { run(); }} {}

  ~TestWatchdog() {
    {
      std::lock_guard lock{mutex_};
      done_ = true;
    }
    cv_.notify_one();
    thread_.join();
  }

 private:
  void run() {
    std::unique_lock lock{mutex_};
    if (cv_.wait_for(lock, std::chrono::seconds{5}, [this] { return done_; })) {
      return;
    }
    std::fprintf(
        stderr,
        "channel_pipeline integration watchdog: operation '%s' exceeded 5 seconds\n",
        operation_);
    std::fflush(stderr);
    std::abort();
  }

  const char* operation_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool done_{false};
  std::thread thread_;
};

struct RejectedAdapter {};

static_assert(!RustMessageAdapterConcept<RejectedAdapter>);

TEST(AdapterTest, BytesPtrAdapterConforms) {
  TestWatchdog watchdog{"BytesPtr adapter take/box round trip"};
  // Verify concept satisfaction at compile time via static_assert in header.
  // Runtime test verifies take/box round trip preserves content.
  auto iobuf = folly::IOBuf::create(10);
  iobuf->append(10);
  std::memset(iobuf->writableData(), 0xAB, 10);

  BytesPtr original = std::move(iobuf);
  auto* originalPtr = original.get();

  auto boxed = RustMessageAdapter<BytesPtr>::tryBox(std::move(original));
  ASSERT_TRUE(boxed.has_value());
  EXPECT_FALSE(boxed->empty());

  auto recovered = RustMessageAdapter<BytesPtr>::tryTake(std::move(*boxed));
  ASSERT_TRUE(recovered.has_value());
  EXPECT_EQ(recovered->get(), originalPtr);
  EXPECT_EQ((*recovered)->length(), 10);
  EXPECT_EQ((*recovered)->data()[0], 0xAB);
}

TEST(AdapterTest, BytesPtrAdapterRestoresOriginalBox) {
  TestWatchdog watchdog{"BytesPtr adapter original-box restoration"};
  TypeErasedBox box;
  auto iobuf = folly::IOBuf::create(4);
  ASSERT_NE(iobuf, nullptr);
  iobuf->append(4);
  auto* originalPtr = iobuf.get();

  EXPECT_TRUE(RustMessageAdapter<BytesPtr>::tryRestore(box, std::move(iobuf)));
  EXPECT_EQ(box.get<BytesPtr>().get(), originalPtr);

  auto replacement = folly::IOBuf::create(1);
  ASSERT_NE(replacement, nullptr);
  replacement->append(1);
  EXPECT_FALSE(
      RustMessageAdapter<BytesPtr>::tryRestore(box, std::move(replacement)));
}

TEST(AdapterTest, BytesPtrAdapterRejectsNullRestore) {
  TestWatchdog watchdog{"null BytesPtr original-box restoration failure"};
  TypeErasedBox box;
  EXPECT_FALSE(RustMessageAdapter<BytesPtr>::tryRestore(box, nullptr));
  EXPECT_TRUE(box.empty());
}

TEST(AdapterTest, BytesPtrAdapterPreservesChain) {
  TestWatchdog watchdog{
      "BytesPtr chain preservation through adapter round trip"};
  // Verify IOBuf chain is preserved through adapter round trip.
  auto head = folly::IOBuf::create(5);
  head->append(5);
  auto tail = folly::IOBuf::create(5);
  tail->append(5);
  head->prependChain(std::move(tail));

  BytesPtr original = std::move(head);
  EXPECT_EQ(original->computeChainDataLength(), 10);
  EXPECT_EQ(original->countChainElements(), 2);

  auto boxed = RustMessageAdapter<BytesPtr>::tryBox(std::move(original));
  ASSERT_TRUE(boxed.has_value());
  auto recovered = RustMessageAdapter<BytesPtr>::tryTake(std::move(*boxed));
  ASSERT_TRUE(recovered.has_value());

  EXPECT_EQ((*recovered)->computeChainDataLength(), 10);
  EXPECT_EQ((*recovered)->countChainElements(), 2);
}

TEST(AdapterTest, EmptyBoxReportsAdapterFailure) {
  TestWatchdog watchdog{"empty box adapter failure"};
  TypeErasedBox box;
  EXPECT_FALSE(
      RustMessageAdapter<BytesPtr>::tryTake(std::move(box)).has_value());
}

TEST(AdapterTest, WrongMessageTypeReportsAdapterFailure) {
  TestWatchdog watchdog{"wrong message type adapter failure"};
  auto box = erase_and_box(uint32_t{42});
  EXPECT_FALSE(
      RustMessageAdapter<BytesPtr>::tryTake(std::move(box)).has_value());
}

TEST(AdapterTest, NullBytesReportsConversionFailure) {
  TestWatchdog watchdog{"null BytesPtr conversion failure"};
  EXPECT_FALSE(RustMessageAdapter<BytesPtr>::tryBox(nullptr).has_value());
}

} // namespace
} // namespace channel_pipeline_rust
