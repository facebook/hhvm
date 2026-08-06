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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/CallbackContext.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustMessageAdapter.h>

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <future>
#include <mutex>
#include <thread>
#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/Request.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

namespace channel_pipeline_rust {
namespace {

using apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl;
using apache::thrift::fast_thrift::channel_pipeline::test::MockHandler;
using apache::thrift::fast_thrift::channel_pipeline::test::MockHeadHandler;
using apache::thrift::fast_thrift::channel_pipeline::test::MockTailHandler;
using apache::thrift::fast_thrift::channel_pipeline::test::TestAllocator;

HANDLER_TAG(context_handle_safety);

struct ContextFixture {
  ContextFixture() {
    auto handler = std::make_unique<MockHandler>();
    handler->setHandlerRemoved([this](ContextImpl&) {
      removed.fetch_add(1, std::memory_order_relaxed);
    });
    eventBase->runInEventBaseThreadAndWait([&] {
      pipeline =
          PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
              .setEventBase(eventBase)
              .setHead(&head)
              .setTail(&tail)
              .setAllocator(&allocator)
              .addNextDuplex<MockHandler>(
                  context_handle_safety_tag, std::move(handler))
              .build();
    });
  }

  folly::ScopedEventBaseThread eventBaseThread;
  folly::EventBase* eventBase{eventBaseThread.getEventBase()};
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  std::atomic<uint32_t> removed{0};
  PipelineImpl::Ptr pipeline;

  ContextImpl& context() {
    return *pipeline->context(context_handle_safety_tag);
  }

  ~ContextFixture() {
    eventBase->runInEventBaseThreadAndWait(
        [pipeline = std::move(pipeline)]() mutable { pipeline.reset(); });
  }
};

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

TEST(ContextHandleSafetyDeathTest, NullConstructionStorage) {
  TestWatchdog watchdog{"null ContextHandle construction storage"};
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  ContextFixture fixture;
  CallbackContext context{fixture.context()};
  EXPECT_DEATH(
      fixture.eventBase->runInEventBaseThreadAndWait(
          [&] { context.initContextHandle(nullptr); }),
      "storage != nullptr");
}

TEST(ContextHandleSafetyDeathTest, MisalignedConstructionStorage) {
  TestWatchdog watchdog{"misaligned ContextHandle construction storage"};
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  ContextFixture fixture;
  CallbackContext context{fixture.context()};
  alignas(void*) std::array<uint8_t, 2 * sizeof(void*) + 1> storage{};
  EXPECT_DEATH(
      fixture.eventBase->runInEventBaseThreadAndWait(
          [&] { context.initContextHandle(storage.data() + 1); }),
      "alignof\\(RustContextHandleToken\\)");
}

TEST(ContextHandleSafetyDeathTest, ConstructionOutsideEventBase) {
  TestWatchdog watchdog{"ContextHandle construction outside EventBase"};
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  ContextFixture fixture;
  CallbackContext context{fixture.context()};
  alignas(void*) std::array<uint8_t, 2 * sizeof(void*)> storage{};
  EXPECT_DEATH(
      context.initContextHandle(storage.data()), "isInEventBaseThread");
}

TEST(ContextHandleSafetyDeathTest, NullDestructionStorage) {
  TestWatchdog watchdog{"null ContextHandle destruction storage"};
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(destroyContextHandle(nullptr), "storage != nullptr");
}

TEST(ContextHandleSafetyDeathTest, MisalignedDestructionStorage) {
  TestWatchdog watchdog{"misaligned ContextHandle destruction storage"};
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  alignas(void*) std::array<uint8_t, 2 * sizeof(void*) + 1> storage{};
  EXPECT_DEATH(
      destroyContextHandle(storage.data() + 1),
      "alignof\\(RustContextHandleToken\\)");
}

TEST(ContextHandleExceptionTest, InlinePreservesAndRestoresRequestContext) {
  TestWatchdog watchdog{"inline exception request context"};
  ContextFixture fixture;
  auto customContext = std::make_shared<folly::RequestContext>();
  auto replacementContext = std::make_shared<folly::RequestContext>();
  bool observedCustomContext = false;
  bool restoredCustomContext = false;

  fixture.eventBase->runInEventBaseThreadAndWait([&] {
    fixture.tail.setOnExceptionCallback(
        [&](folly::exception_wrapper&&) noexcept {
          observedCustomContext =
              folly::RequestContext::get() == customContext.get();
          folly::RequestContext::setContext(replacementContext);
        });
    folly::RequestContextScopeGuard contextGuard{customContext};
    CallbackContext context{fixture.context()};
    alignas(void*) std::array<uint8_t, 2 * sizeof(void*)> storage{};
    context.initContextHandle(storage.data());
    constexpr std::string_view message{"inline context"};
    fireContextHandleException(
        storage.data(),
        reinterpret_cast<const uint8_t*>(message.data()),
        message.size());
    restoredCustomContext = folly::RequestContext::get() == customContext.get();
  });

  EXPECT_TRUE(observedCustomContext);
  EXPECT_TRUE(restoredCustomContext);
}

TEST(ContextHandleExceptionTest, WorkerCapturesRequestContext) {
  TestWatchdog watchdog{"worker exception request context"};
  ContextFixture fixture;
  auto customContext = std::make_shared<folly::RequestContext>();
  std::atomic<bool> observedCustomContext{false};
  alignas(void*) std::array<uint8_t, 2 * sizeof(void*)> storage{};

  fixture.eventBase->runInEventBaseThreadAndWait([&] {
    fixture.tail.setOnExceptionCallback(
        [&](folly::exception_wrapper&&) noexcept {
          observedCustomContext.store(
              folly::RequestContext::get() == customContext.get(),
              std::memory_order_relaxed);
        });
    CallbackContext context{fixture.context()};
    context.initContextHandle(storage.data());
  });

  std::thread worker([&] {
    folly::RequestContextScopeGuard contextGuard{customContext};
    constexpr std::string_view message{"worker context"};
    fireContextHandleException(
        storage.data(),
        reinterpret_cast<const uint8_t*>(message.data()),
        message.size());
  });
  worker.join();
  fixture.eventBase->runInEventBaseThreadAndWait([] {});

  EXPECT_TRUE(observedCustomContext.load(std::memory_order_relaxed));
}

TEST(ContextHandleExceptionTest, DistinctHandlesFireConcurrentlyExactlyOnce) {
  TestWatchdog watchdog{"concurrent exception handles"};
  ContextFixture fixture;
  std::atomic<uint32_t> exceptions{0};
  alignas(void*) std::array<uint8_t, 2 * sizeof(void*)> first{};
  alignas(void*) std::array<uint8_t, 2 * sizeof(void*)> second{};

  fixture.eventBase->runInEventBaseThreadAndWait([&] {
    fixture.tail.setOnExceptionCallback(
        [&](folly::exception_wrapper&&) noexcept {
          exceptions.fetch_add(1, std::memory_order_relaxed);
        });
    CallbackContext firstContext{fixture.context()};
    firstContext.initContextHandle(first.data());
    CallbackContext secondContext{fixture.context()};
    secondContext.initContextHandle(second.data());
  });

  auto fire = [](auto& storage, std::string_view message) {
    fireContextHandleException(
        storage.data(),
        reinterpret_cast<const uint8_t*>(message.data()),
        message.size());
  };
  std::thread firstWorker([&] { fire(first, "first"); });
  std::thread secondWorker([&] { fire(second, "second"); });
  firstWorker.join();
  secondWorker.join();
  fixture.eventBase->runInEventBaseThreadAndWait([] {});

  EXPECT_EQ(exceptions.load(std::memory_order_relaxed), 2);
}

TEST(ContextHandleSafetyDeathTest, LiveHandleDestructionOutsideEventBase) {
  TestWatchdog watchdog{"live ContextHandle destruction outside EventBase"};
  ContextFixture fixture;
  CallbackContext context{fixture.context()};
  alignas(void*) std::array<uint8_t, 2 * sizeof(void*)> storage{};
  fixture.eventBase->runInEventBaseThreadAndWait(
      [&] { context.initContextHandle(storage.data()); });

  std::promise<void> blockerStarted;
  std::promise<void> unblock;
  auto unblockFuture = unblock.get_future().share();
  fixture.eventBase->runInEventBaseThread([&blockerStarted, unblockFuture] {
    blockerStarted.set_value();
    unblockFuture.wait();
  });
  blockerStarted.get_future().wait();

  destroyContextHandle(storage.data());
  fixture.pipeline.reset();
  EXPECT_EQ(fixture.removed.load(std::memory_order_relaxed), 0);

  unblock.set_value();
  fixture.eventBase->runInEventBaseThreadAndWait([] {});
  EXPECT_EQ(fixture.removed.load(std::memory_order_relaxed), 1);
}

} // namespace
} // namespace channel_pipeline_rust
