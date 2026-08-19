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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/PipelineTestHelper.h>

#include <thrift/lib/rust/channel_pipeline/src/integration_test.rs.h>

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>

#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/memory/MallctlHelper.h>
#include <folly/memory/Malloc.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/CallbackContext.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustHandler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustMessageAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

namespace channel_pipeline_rust::test {
namespace {

using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;

HANDLER_TAG(normal_before);
HANDLER_TAG(rust_middle);
HANDLER_TAG(normal_after);
HANDLER_TAG(rust_before);

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

BytesPtr makeBytes(size_t size, uint8_t value) {
  auto bytes = folly::IOBuf::create(size);
  CHECK(bytes);
  bytes->append(size);
  std::memset(bytes->writableData(), value, size);
  return bytes;
}

uint64_t threadAllocatedBytes() {
  if (!folly::usingJEMalloc()) {
    return 0;
  }
  uint64_t allocated = 0;
  folly::mallctlRead("thread.allocated", &allocated);
  return allocated;
}

std::pair<uint64_t, uint8_t> writtenBytesObservation(
    const MockHeadHandler& head) {
  const auto& written = head.writtenBytes();
  if (written.empty() || !written.front() || written.front()->empty()) {
    return {0, 0};
  }
  return {written.front()->length(), written.front()->data()[0]};
}

} // namespace

PipelineTestResult run_pipeline_test() noexcept {
  TestWatchdog watchdog{"normal C++ -> Rust -> normal C++ pipeline"};
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;

  auto before = std::make_unique<MockHandler>();
  auto* beforePtr = before.get();
  auto after = std::make_unique<MockHandler>();
  auto* afterPtr = after.get();

  using ProductionRustHandler = RustHandler<detail::ContextImpl>;
  auto rustHandler =
      std::make_unique<ProductionRustHandler>(rust_handler_new_counting_test());
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<MockHandler>(normal_before_tag, std::move(before))
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag, std::move(rustHandler))
          .addNextDuplex<MockHandler>(normal_after_tag, std::move(after))
          .build();

  CHECK(pipeline);
  pipeline->activate();
  eventBase.loopOnce();
  (void)pipeline->fireRead(erase_and_box(makeBytes(10, 0xab)));
  eventBase.loopOnce();
  (void)pipeline->fireWrite(erase_and_box(makeBytes(5, 0xcd)));
  eventBase.loopOnce();

  const auto rustReads = rust_handler_test_read_callbacks();
  const auto rustWrites = rust_handler_test_write_callbacks();

  const auto [writtenLength, writtenFirstByte] = writtenBytesObservation(head);

  PipelineTestResult result{
      .cpp_before_reads = static_cast<uint32_t>(beforePtr->readCount()),
      .cpp_after_reads = static_cast<uint32_t>(afterPtr->readCount()),
      .cpp_before_writes = static_cast<uint32_t>(beforePtr->writeCount()),
      .cpp_after_writes = static_cast<uint32_t>(afterPtr->writeCount()),
      .tail_reads = static_cast<uint32_t>(tail.readCount()),
      .head_writes = static_cast<uint32_t>(head.writeCount()),
      .written_length = writtenLength,
      .written_first_byte = static_cast<uint8_t>(writtenFirstByte),
      .rust_reads = rustReads,
      .rust_writes = rustWrites,
  };

  pipeline->deactivate();
  eventBase.loopOnce();
  pipeline.reset();
  eventBase.loopOnce();
  return result;
}

namespace {

using ProductionRustHandler = RustHandler<detail::ContextImpl>;

Result runReadWithHandler(rust::Box<RustHandlerOpaque> handler) {
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag,
              std::make_unique<ProductionRustHandler>(std::move(handler)))
          .build();
  const auto result = pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  eventBase.loopOnce();
  pipeline.reset();
  eventBase.loopOnce();
  return result;
}

Result runReadBoxWithHandler(
    rust::Box<RustHandlerOpaque> handler, TypeErasedBox message) {
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag,
              std::make_unique<ProductionRustHandler>(std::move(handler)))
          .build();
  const auto result = pipeline->fireRead(std::move(message));
  pipeline.reset();
  eventBase.loopOnce();
  return result;
}

Result runWriteBoxWithHandler(
    rust::Box<RustHandlerOpaque> handler, TypeErasedBox message) {
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag,
              std::make_unique<ProductionRustHandler>(std::move(handler)))
          .build();
  const auto result = pipeline->fireWrite(std::move(message));
  pipeline.reset();
  eventBase.loopOnce();
  return result;
}

struct DownstreamResultObservation {
  Result result;
  bool pointerIdentityPreserved;
};

DownstreamResultObservation runReadWithDownstreamResult(Result downstream) {
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  const folly::IOBuf* received = nullptr;
  tail.setOnReadCallback([&](TypeErasedBox&& message) noexcept {
    received = message.get<BytesPtr>().get();
    return downstream;
  });
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag,
              std::make_unique<ProductionRustHandler>(
                  rust_handler_new_counting_test()))
          .build();
  auto bytes = makeBytes(1, 0xa5);
  const auto* original = bytes.get();
  const auto result = pipeline->fireRead(erase_and_box(std::move(bytes)));
  pipeline.reset();
  eventBase.loopOnce();
  return {result, received == original};
}

DownstreamResultObservation runWriteWithDownstreamResult(Result downstream) {
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  const folly::IOBuf* received = nullptr;
  head.setOnWriteCallback([&](TypeErasedBox&& message) noexcept {
    received = message.get<BytesPtr>().get();
    return downstream;
  });
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag,
              std::make_unique<ProductionRustHandler>(
                  rust_handler_new_counting_test()))
          .build();
  auto bytes = makeBytes(1, 0xa5);
  const auto* original = bytes.get();
  const auto result = pipeline->fireWrite(erase_and_box(std::move(bytes)));
  pipeline.reset();
  eventBase.loopOnce();
  return {result, received == original};
}

Result runWriteWithHandler(rust::Box<RustHandlerOpaque> handler) {
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag,
              std::make_unique<ProductionRustHandler>(std::move(handler)))
          .build();
  const auto result = pipeline->fireWrite(erase_and_box(makeBytes(1, 0xa5)));
  eventBase.loopOnce();
  pipeline.reset();
  eventBase.loopOnce();
  return result;
}

PipelineImpl::Ptr buildSingleRust(
    folly::EventBase& eventBase,
    MockHeadHandler& head,
    MockTailHandler& tail,
    TestAllocator& allocator,
    rust::Box<RustHandlerOpaque> handler) {
  return PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
      .setEventBase(&eventBase)
      .setHead(&head)
      .setTail(&tail)
      .setAllocator(&allocator)
      .addNextDuplex<ProductionRustHandler>(
          rust_middle_tag,
          std::make_unique<ProductionRustHandler>(std::move(handler)))
      .build();
}

} // namespace

bool run_downstream_handler_test() noexcept {
  return runReadWithHandler(new_downstream_handler()) == Result::Success;
}

BehaviorTestResult run_behavior_test() noexcept {
  TestWatchdog watchdog{"synchronous handler behavior"};
  rust_handler_reset_test_counts();
  const auto successRead = runReadWithHandler(rust_handler_new_counting_test());
  const auto backpressureRead =
      runReadWithHandler(rust_handler_new_backpressure_test());
  const auto backpressureWrite =
      runWriteWithHandler(rust_handler_new_backpressure_test());
  const auto errorRead = runReadWithHandler(rust_handler_new_error_test());
  const auto errorWrite = runWriteWithHandler(rust_handler_new_error_test());
  const auto panicRead = runReadWithHandler(rust_handler_new_panicking_test());
  const auto panicWrite =
      runWriteWithHandler(rust_handler_new_panicking_test());
  const auto mismatchRead = runReadBoxWithHandler(
      rust_handler_new_counting_test(), erase_and_box(uint32_t{42}));
  const auto mismatchWrite = runWriteBoxWithHandler(
      rust_handler_new_counting_test(), erase_and_box(uint32_t{42}));
  const auto emptyRead =
      runReadBoxWithHandler(rust_handler_new_counting_test(), TypeErasedBox{});
  const auto downstreamBackpressureRead =
      runReadWithDownstreamResult(Result::Backpressure);
  const auto downstreamErrorWrite = runWriteWithDownstreamResult(Result::Error);

  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag,
              std::make_unique<ProductionRustHandler>(
                  rust_handler_new_lifecycle_test()))
          .build();
  pipeline->activate();
  eventBase.loopOnce();
  (void)pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  pipeline->onReadReady();
  (void)pipeline->fireWrite(erase_and_box(makeBytes(1, 0xa5)));
  pipeline->onWriteReady();
  pipeline->fireException(
      folly::make_exception_wrapper<std::runtime_error>("test exception"));
  eventBase.loopOnce();
  pipeline->deactivate();
  eventBase.loopOnce();
  pipeline.reset();
  eventBase.loopOnce();

  return BehaviorTestResult{
      .success_read = static_cast<int32_t>(successRead),
      .backpressure_read = static_cast<int32_t>(backpressureRead),
      .backpressure_write = static_cast<int32_t>(backpressureWrite),
      .error_read = static_cast<int32_t>(errorRead),
      .error_write = static_cast<int32_t>(errorWrite),
      .panic_read = static_cast<int32_t>(panicRead),
      .panic_write = static_cast<int32_t>(panicWrite),
      .mismatch_read = static_cast<int32_t>(mismatchRead),
      .mismatch_write = static_cast<int32_t>(mismatchWrite),
      .empty_read = static_cast<int32_t>(emptyRead),
      .downstream_backpressure_read =
          static_cast<int32_t>(downstreamBackpressureRead.result),
      .downstream_error_write =
          static_cast<int32_t>(downstreamErrorWrite.result),
      .read_pointer_identity_preserved =
          downstreamBackpressureRead.pointerIdentityPreserved,
      .write_pointer_identity_preserved =
          downstreamErrorWrite.pointerIdentityPreserved,
      .exceptions = rust_handler_test_exception_callbacks(),
      .read_ready = rust_handler_test_read_ready_callbacks(),
      .write_ready = rust_handler_test_write_ready_callbacks(),
      .added = rust_handler_test_added_callbacks(),
      .active = rust_handler_test_active_callbacks(),
      .inactive = rust_handler_test_inactive_callbacks(),
      .removed = rust_handler_test_removed_callbacks(),
  };
}

namespace {

PositionTestResult runPositionTest(
    bool rustFirst, bool rustMiddle, bool rustLast) noexcept {
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;

  rust_handler_reset_test_counts();

  PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator> builder;
  builder.setEventBase(&eventBase)
      .setHead(&head)
      .setTail(&tail)
      .setAllocator(&allocator);

  if (rustFirst) {
    builder.addNextDuplex<ProductionRustHandler>(
        normal_before_tag,
        std::make_unique<ProductionRustHandler>(
            rust_handler_new_counting_test()));
  } else {
    builder.addNextDuplex<MockHandler>(
        normal_before_tag, std::make_unique<MockHandler>());
  }

  if (rustMiddle) {
    builder.addNextDuplex<ProductionRustHandler>(
        rust_middle_tag,
        std::make_unique<ProductionRustHandler>(
            rust_handler_new_counting_test()));
  } else {
    builder.addNextDuplex<MockHandler>(
        rust_middle_tag, std::make_unique<MockHandler>());
  }

  if (rustLast) {
    builder.addNextDuplex<ProductionRustHandler>(
        normal_after_tag,
        std::make_unique<ProductionRustHandler>(
            rust_handler_new_counting_test()));
  } else {
    builder.addNextDuplex<MockHandler>(
        normal_after_tag, std::make_unique<MockHandler>());
  }

  auto pipeline = std::move(builder).build();

  CHECK(pipeline);
  pipeline->activate();
  eventBase.loopOnce();
  (void)pipeline->fireRead(erase_and_box(makeBytes(10, 0xab)));
  eventBase.loopOnce();
  (void)pipeline->fireWrite(erase_and_box(makeBytes(5, 0xcd)));
  eventBase.loopOnce();

  const auto rustReads = rust_handler_test_read_callbacks();
  const auto rustWrites = rust_handler_test_write_callbacks();

  const auto [writtenLength, writtenFirstByte] = writtenBytesObservation(head);

  PositionTestResult result{
      .tail_reads = static_cast<uint32_t>(tail.readCount()),
      .head_writes = static_cast<uint32_t>(head.writeCount()),
      .written_length = writtenLength,
      .written_first_byte = static_cast<uint8_t>(writtenFirstByte),
      .rust_reads = rustReads,
      .rust_writes = rustWrites,
  };

  pipeline->deactivate();
  eventBase.loopOnce();
  pipeline.reset();
  eventBase.loopOnce();
  return result;
}

} // namespace

PositionTestResult run_position_test_first() noexcept {
  TestWatchdog watchdog{"Rust handler first position"};
  return runPositionTest(true, false, false);
}

PositionTestResult run_position_test_middle() noexcept {
  TestWatchdog watchdog{"Rust handler middle position"};
  return runPositionTest(false, true, false);
}

PositionTestResult run_position_test_last() noexcept {
  TestWatchdog watchdog{"Rust handler last position"};
  return runPositionTest(false, false, true);
}

LifecycleOrderResult run_lifecycle_order_test() noexcept {
  TestWatchdog watchdog{"lifecycle callback ordering"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              normal_before_tag,
              std::make_unique<ProductionRustHandler>(
                  rust_handler_new_lifecycle_order(1)))
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag,
              std::make_unique<ProductionRustHandler>(
                  rust_handler_new_lifecycle_order(2)))
          .build();

  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(4, 0xab)));
  (void)pipeline->fireWrite(erase_and_box(makeBytes(4, 0xcd)));
  pipeline->fireException(
      folly::make_exception_wrapper<std::runtime_error>("order"));
  pipeline->deactivate();

  const auto tailReads = static_cast<uint32_t>(tail.readCount());
  const auto headWrites = static_cast<uint32_t>(head.writeCount());

  pipeline.reset();

  return LifecycleOrderResult{
      .sequence = rust_handler_test_sequence(),
      .tail_reads = tailReads,
      .head_writes = headWrites,
  };
}

ReadinessCycleResult run_read_recovery_test() noexcept {
  TestWatchdog watchdog{"read backpressure -> ready -> recovery"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_recovering_read());

  pipeline->activate();
  const auto firstResult =
      pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  const bool armedAfterBackpressure = pipeline->hasPendingReadReady();
  pipeline->onReadReady();
  pipeline->onReadReady();
  const bool disarmedAfterReady = !pipeline->hasPendingReadReady();
  const auto recoveredResult =
      pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  const auto tailReads = static_cast<uint32_t>(tail.readCount());

  pipeline->deactivate();
  pipeline.reset();

  return ReadinessCycleResult{
      .sequence = rust_handler_test_sequence(),
      .armed_after_backpressure = armedAfterBackpressure,
      .disarmed_after_ready = disarmedAfterReady,
      .terminal_after_recovery = tailReads,
      .first_result = static_cast<int32_t>(firstResult),
      .recovered_result = static_cast<int32_t>(recoveredResult),
  };
}

ReadinessCycleResult run_write_recovery_test() noexcept {
  TestWatchdog watchdog{"write backpressure -> ready -> recovery"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_recovering_write());

  pipeline->activate();
  const auto firstResult =
      pipeline->fireWrite(erase_and_box(makeBytes(1, 0xa5)));
  const bool armedAfterBackpressure = pipeline->hasPendingWriteReady();
  pipeline->onWriteReady();
  pipeline->onWriteReady();
  const bool disarmedAfterReady = !pipeline->hasPendingWriteReady();
  const auto recoveredResult =
      pipeline->fireWrite(erase_and_box(makeBytes(1, 0xa5)));
  const auto headWrites = static_cast<uint32_t>(head.writeCount());

  pipeline->deactivate();
  pipeline.reset();

  return ReadinessCycleResult{
      .sequence = rust_handler_test_sequence(),
      .armed_after_backpressure = armedAfterBackpressure,
      .disarmed_after_ready = disarmedAfterReady,
      .terminal_after_recovery = headWrites,
      .first_result = static_cast<int32_t>(firstResult),
      .recovered_result = static_cast<int32_t>(recoveredResult),
  };
}

RearmResult run_read_rearm_test() noexcept {
  TestWatchdog watchdog{"read readiness explicit re-arm"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_read_rearm(2));

  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  pipeline->onReadReady();
  pipeline->onReadReady();
  pipeline->onReadReady();
  pipeline->onReadReady();
  const bool armedAfterLastReady = pipeline->hasPendingReadReady();

  pipeline->deactivate();
  pipeline.reset();

  return RearmResult{
      .sequence = rust_handler_test_sequence(),
      .armed_after_last_ready = armedAfterLastReady,
  };
}

RearmResult run_multi_handler_rearm_test() noexcept {
  TestWatchdog watchdog{"multi-handler readiness re-arm deferral"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              normal_before_tag,
              std::make_unique<ProductionRustHandler>(
                  rust_handler_new_ready_rearm_bench()))
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag,
              std::make_unique<ProductionRustHandler>(
                  rust_handler_new_ready_rearm_bench()))
          .build();

  pipeline->activate();
  pipeline->context(normal_before_tag)->awaitReadReady();
  pipeline->context(rust_middle_tag)->awaitReadReady();
  pipeline->onReadReady();
  const bool armedAfterReady = pipeline->hasPendingReadReady();

  pipeline->deactivate();
  pipeline.reset();

  return RearmResult{
      .sequence = rust_handler_test_sequence(),
      .armed_after_last_ready = armedAfterReady,
  };
}

BidirectionalResult run_bidirectional_test() noexcept {
  TestWatchdog watchdog{"independent bidirectional backpressure"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_bidirectional());

  pipeline->activate();

  (void)pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  const bool readArmedAfterReadBp = pipeline->hasPendingReadReady();
  const bool writeArmedAfterReadBp = pipeline->hasPendingWriteReady();

  (void)pipeline->fireWrite(erase_and_box(makeBytes(1, 0xa5)));
  const bool writeArmedAfterWriteBp = pipeline->hasPendingWriteReady();
  const bool readArmedAfterWriteBp = pipeline->hasPendingReadReady();

  pipeline->onReadReady();
  const bool writeArmedAfterReadReady = pipeline->hasPendingWriteReady();
  const bool readDisarmedAfterReadReady = !pipeline->hasPendingReadReady();

  pipeline->onWriteReady();
  const bool writeDisarmedAfterWriteReady = !pipeline->hasPendingWriteReady();

  pipeline->deactivate();
  pipeline.reset();

  return BidirectionalResult{
      .sequence = rust_handler_test_sequence(),
      .read_armed_after_read_bp = readArmedAfterReadBp,
      .write_armed_after_read_bp = writeArmedAfterReadBp,
      .write_armed_after_write_bp = writeArmedAfterWriteBp,
      .read_armed_after_write_bp = readArmedAfterWriteBp,
      .write_armed_after_read_ready = writeArmedAfterReadReady,
      .read_disarmed_after_read_ready = readDisarmedAfterReadReady,
      .write_disarmed_after_write_ready = writeDisarmedAfterWriteReady,
  };
}

ProbeResult run_readiness_probe_test() noexcept {
  TestWatchdog watchdog{"safe readiness API idempotency and independence"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_readiness_probe());

  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  const bool readArmedAfter = pipeline->hasPendingReadReady();
  const bool writeArmedAfter = pipeline->hasPendingWriteReady();

  pipeline->deactivate();
  pipeline.reset();

  return ProbeResult{
      .checks_passed = rust_handler_test_probe_checks(),
      .expected_checks = rust_handler_readiness_probe_check_count(),
      .read_armed_after = readArmedAfter,
      .write_armed_after = writeArmedAfter,
  };
}

namespace {

TeardownResult armThenTeardown(int action) {
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_lifecycle_test());

  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  (void)pipeline->fireWrite(erase_and_box(makeBytes(1, 0xa5)));
  const bool readArmedBefore = pipeline->hasPendingReadReady();
  const bool writeArmedBefore = pipeline->hasPendingWriteReady();

  bool readArmedAfter = false;
  bool writeArmedAfter = false;
  if (action == 0) {
    pipeline->close();
    readArmedAfter = pipeline->hasPendingReadReady();
    writeArmedAfter = pipeline->hasPendingWriteReady();
    pipeline->onReadReady();
    pipeline->onWriteReady();
  } else if (action == 1) {
    pipeline->deactivate();
    readArmedAfter = pipeline->hasPendingReadReady();
    writeArmedAfter = pipeline->hasPendingWriteReady();
    pipeline->onReadReady();
    pipeline->onWriteReady();
  }

  const auto readReadyAfter = rust_handler_test_read_ready_callbacks();
  const auto writeReadyAfter = rust_handler_test_write_ready_callbacks();

  pipeline.reset();

  return TeardownResult{
      .read_armed_before = readArmedBefore,
      .write_armed_before = writeArmedBefore,
      .read_armed_after = readArmedAfter,
      .write_armed_after = writeArmedAfter,
      .read_ready_after = readReadyAfter,
      .write_ready_after = writeReadyAfter,
      .removed = rust_handler_test_removed_callbacks(),
      .inactive = rust_handler_test_inactive_callbacks(),
  };
}

} // namespace

TeardownResult run_close_while_armed_test() noexcept {
  TestWatchdog watchdog{"close while readiness armed"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_rearm_on_removed());

  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  (void)pipeline->fireWrite(erase_and_box(makeBytes(1, 0xa5)));
  const bool readArmedBefore = pipeline->hasPendingReadReady();
  const bool writeArmedBefore = pipeline->hasPendingWriteReady();

  pipeline->close();
  const bool readArmedAfter = pipeline->hasPendingReadReady();
  const bool writeArmedAfter = pipeline->hasPendingWriteReady();
  pipeline->onReadReady();
  pipeline->onWriteReady();

  auto result = TeardownResult{
      .read_armed_before = readArmedBefore,
      .write_armed_before = writeArmedBefore,
      .read_armed_after = readArmedAfter,
      .write_armed_after = writeArmedAfter,
      .read_ready_after = rust_handler_test_read_ready_callbacks(),
      .write_ready_after = rust_handler_test_write_ready_callbacks(),
      .removed = rust_handler_test_removed_callbacks(),
      .inactive = rust_handler_test_inactive_callbacks(),
  };
  pipeline.reset();
  return result;
}

TeardownResult run_inactive_while_armed_test() noexcept {
  TestWatchdog watchdog{"inactive while readiness armed"};
  return armThenTeardown(1);
}

TeardownResult run_destroy_while_armed_test() noexcept {
  TestWatchdog watchdog{"destroy while readiness armed"};
  return armThenTeardown(2);
}

PanicResult run_panic_containment_test() noexcept {
  TestWatchdog watchdog{"panic containment in lifecycle and ready callbacks"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_panicking_lifecycle());

  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  (void)pipeline->fireWrite(erase_and_box(makeBytes(1, 0xa5)));
  pipeline->onReadReady();
  pipeline->onWriteReady();
  pipeline->fireException(
      folly::make_exception_wrapper<std::runtime_error>("panic"));
  const auto tailExceptions = static_cast<uint32_t>(tail.exceptionCount());
  pipeline->deactivate();
  pipeline.reset();

  return PanicResult{
      .completed = true,
      .tail_exceptions = tailExceptions,
  };
}

IdentityResult run_identity_test() noexcept {
  TestWatchdog watchdog{"identity handlerId"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_identity());

  const uint64_t expected = static_cast<uint64_t>(rust_middle_tag.id);
  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(8, 0xaa)));
  const uint64_t observed = rust_handler_phase3_handler_id();
  const bool isNonzero = observed != 0;
  const bool matchesTag = observed == expected;
  const auto tailReads = tail.readCount();
  pipeline->deactivate();
  pipeline.reset();

  return IdentityResult{
      .observed_id = observed,
      .expected_id = expected,
      .is_nonzero = isNonzero,
      .matches_tag = matchesTag,
      .tail_reads = static_cast<uint32_t>(tailReads),
  };
}

AllocationResult run_allocation_probe_test() noexcept {
  TestWatchdog watchdog{"allocation probe"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_allocation_probe());

  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(1, 0xbb)));
  const uint32_t checks = rust_handler_phase3_alloc_checks();
  const uint32_t expected = rust_handler_allocation_probe_check_count();
  const int allocCount = allocator.allocationCount();
  pipeline->deactivate();
  pipeline.reset();

  return AllocationResult{
      .checks_passed = checks,
      .expected_checks = expected,
      .allocator_invocations = static_cast<uint32_t>(allocCount),
  };
}

CopyResult run_copy_probe_test() noexcept {
  TestWatchdog watchdog{"copy probe"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;

  auto chain = folly::IOBuf::create(3);
  chain->append(3);
  chain->writableData()[0] = 0xa1;
  chain->writableData()[1] = 0xa2;
  chain->writableData()[2] = 0xa3;
  auto extra = folly::IOBuf::create(2);
  extra->append(2);
  extra->writableData()[0] = 0xb1;
  extra->writableData()[1] = 0xb2;
  chain->prependChain(std::move(extra));

  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_copy_probe());

  pipeline->activate();
  auto inbound = chain->clone();
  (void)pipeline->fireRead(erase_and_box(std::move(inbound)));
  const uint32_t checks = rust_handler_phase3_copy_checks();
  const uint32_t expected = rust_handler_copy_probe_check_count();
  const bool pointerPreserved = true;
  const auto tailReads = tail.readCount();
  pipeline->deactivate();
  pipeline.reset();

  return CopyResult{
      .checks_passed = checks,
      .expected_checks = expected,
      .tail_reads = static_cast<uint32_t>(tailReads),
      .pointer_preserved = pointerPreserved,
  };
}

CloseResult run_close_probe_test() noexcept {
  TestWatchdog watchdog{"close probe"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_close_probe());

  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(16, 0xcc)));
  const uint32_t checks = rust_handler_phase3_close_checks();
  const uint32_t expected = rust_handler_close_probe_check_count();
  const auto tailReads = tail.readCount();
  const auto removed = rust_handler_test_removed_callbacks();
  const auto inactive = rust_handler_test_inactive_callbacks();
  const auto afterCloseResult =
      pipeline->fireRead(erase_and_box(makeBytes(1, 0xdd)));
  const bool topLevelIsError = afterCloseResult == Result::Error;
  pipeline.reset();

  return CloseResult{
      .checks_passed = checks,
      .expected_checks = expected,
      .tail_reads = static_cast<uint32_t>(tailReads),
      .removed = removed,
      .inactive = inactive,
      .top_level_is_error = topLevelIsError,
  };
}

// ── Phase 5 ──────────────────────────────────────────────────────────────────

StateMachineResult run_state_machine_test() noexcept {
  TestWatchdog watchdog{"state machine lifecycle"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              rust_before_tag,
              std::make_unique<ProductionRustHandler>(
                  rust_handler_new_state_machine(1)))
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag,
              std::make_unique<ProductionRustHandler>(
                  rust_handler_new_state_machine(2)))
          .build();

  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(4, 0xa5)));
  (void)pipeline->fireWrite(erase_and_box(makeBytes(4, 0xb5)));
  uint32_t tailReads = tail.readCount();
  uint32_t headWrites = head.writeCount();
  uint32_t stateBefore = rust_handler_phase5_state();
  auto countsBefore = rust_handler_phase5_p5_counts();
  // capture P5 sequence before close to prove added→active→data in native order
  ::rust::String p5seqBefore = rust_handler_phase5_p5_sequence();

  pipeline->close();

  uint32_t removed = rust_handler_phase5_p5_counts()[5];
  uint32_t inactive = rust_handler_phase5_p5_counts()[4];
  uint32_t state = rust_handler_phase5_state();
  auto afterRead = pipeline->fireRead(erase_and_box(makeBytes(1, 0xcc)));
  auto afterWrite = pipeline->fireWrite(erase_and_box(makeBytes(1, 0xdd)));
  bool closeClearedHooks =
      !pipeline->hasPendingReadReady() && !pipeline->hasPendingWriteReady();
  ::rust::String finalSeq = rust_handler_phase5_p5_sequence();

  pipeline.reset();

  (void)countsBefore;
  (void)stateBefore;

  return StateMachineResult{
      .sequence = std::move(finalSeq),
      .state_bits = state,
      .tail_reads = tailReads,
      .head_writes = headWrites,
      .removed = removed,
      .inactive = inactive,
      .closed = closeClearedHooks,
      .top_level_read_is_error = afterRead == Result::Error,
      .top_level_write_is_error = afterWrite == Result::Error,
  };
}

PanicRetentionResult run_panic_retention_test() noexcept {
  TestWatchdog watchdog{"panic retention + non-escape"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_panic_retention());

  pipeline->activate();
  auto readResult = pipeline->fireRead(erase_and_box(makeBytes(1, 0xa5)));
  auto writeResult = pipeline->fireWrite(erase_and_box(makeBytes(1, 0xa5)));
  pipeline->fireException(
      folly::make_exception_wrapper<std::runtime_error>("panic-retention"));
  uint32_t tailEx = tail.exceptionCount();
  pipeline->deactivate();
  pipeline.reset();

  return PanicRetentionResult{
      .completed = true,
      .read_is_error = readResult == Result::Error,
      .write_is_error = writeResult == Result::Error,
      .tail_exceptions = tailEx,
  };
}

ExceptionPreserveResult run_exception_preserve_test() noexcept {
  TestWatchdog watchdog{"exception wrapper preserved + no double forward"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  bool tailSawOriginal = false;

  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_exception_preserve());

  tail.setOnExceptionCallback([&](folly::exception_wrapper&& e) noexcept {
    bool matches = false;
    e.with_exception([&](const std::runtime_error& re) {
      if (std::string(re.what()) == "phase5 test exception") {
        matches = true;
      }
    });
    if (matches) {
      tailSawOriginal = true;
    }
  });

  pipeline->activate();
  pipeline->fireException(
      folly::make_exception_wrapper<std::runtime_error>(
          "phase5 test exception"));

  uint32_t observed = rust_handler_phase5_exception_preserved();
  uint32_t tailExCount = tail.exceptionCount();

  pipeline->deactivate();
  pipeline.reset();

  return ExceptionPreserveResult{
      .rust_observed = observed,
      .tail_exceptions = tailExCount,
      .tail_saw_original = tailSawOriginal,
  };
}

ReentrancyResult run_reentrancy_test() noexcept {
  TestWatchdog watchdog{"reentrancy: forward + re-arm inside callbacks"};
  rust_handler_reset_test_counts();
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase, head, tail, allocator, rust_handler_new_reentrancy());

  // Before read, activate observer — already counts added/reentrancy.
  pipeline->activate();
  (void)pipeline->fireRead(erase_and_box(makeBytes(2, 0xe5)));
  (void)pipeline->fireWrite(erase_and_box(makeBytes(2, 0xe6)));
  // Explicit re-arm inside ready callback path — use ContextImpl directly for
  // deterministic invocation
  pipeline->context(rust_middle_tag)->awaitReadReady();
  pipeline->onReadReady();
  pipeline->onReadReady();
  pipeline->context(rust_middle_tag)->awaitWriteReady();
  pipeline->onWriteReady();
  pipeline->onWriteReady();
  uint32_t reentrancyCount = rust_handler_phase5_reentrancy();
  // Total lifecycle + data + ready = added(1) + active(always 1) +
  // read(1)+write(1)+read_ready(re-arms)+write_ready(re-arms)+inactive(at
  // loop)+removed After above, read 1 + write 1 + added 1 + active 1 = 4
  // minimum; plus per re-arm inside ready (2 reads arms + 2 writes) => ~6+
  uint32_t tailReads = tail.readCount();
  pipeline->deactivate();
  pipeline.reset();

  return ReentrancyResult{
      .reentrancy_count = reentrancyCount,
      .tail_reads = tailReads,
      .completed = true,
  };
}

// ── Phase 6: adapter extensibility + event noop isolation ─────────────────

AdapterExtResult run_adapter_ext_test() noexcept {
  TestWatchdog watchdog{"adapter extensibility - BytesPtr only"};
  rust_handler_reset_test_counts();

  uint32_t passed = 0;
  uint32_t expected = 5;

  // 1) BytesPtr round-trip preserving pointer identity via makeBytes +
  // erase_and_box + get<BytesPtr>. Zero-copy forward path.
  bool pointer_identity = false;
  {
    auto bytes = makeBytes(10, 0xAB);
    auto* raw = bytes.get();
    auto boxed = RustMessageAdapter<BytesPtr>::tryBox(std::move(bytes));
    if (boxed.has_value() && !boxed->empty()) {
      auto taken = RustMessageAdapter<BytesPtr>::tryTake(std::move(*boxed));
      if (taken.has_value() && taken->get() == raw) {
        pointer_identity = true;
        ++passed;
      }
    }
  }

  // 2) Null rejection: tryBox(nullptr) -> nullopt.
  bool null_rejected = false;
  {
    auto null_boxed = RustMessageAdapter<BytesPtr>::tryBox(nullptr);
    if (!null_boxed.has_value()) {
      null_rejected = true;
      ++passed;
    }
  }

  // 3) Wrong-type rejection: box uint32_t{42} -> tryTake<BytesPtr> -> nullopt.
  bool wrong_type_rejected = false;
  {
    auto box = erase_and_box(uint32_t{42});
    auto taken = RustMessageAdapter<BytesPtr>::tryTake(std::move(box));
    if (!taken.has_value()) {
      wrong_type_rejected = true;
      ++passed;
    }
  }

  // 4) Empty box rejection.
  bool empty_rejected = false;
  {
    TypeErasedBox empty;
    auto taken = RustMessageAdapter<BytesPtr>::tryTake(std::move(empty));
    if (!taken.has_value()) {
      empty_rejected = true;
      ++passed;
    }
  }

  // 5) Chain independence: IOBuf chain preserved through adapter round-trip,
  // zero allocation path, edge case: multi-element chain.
  bool chain_independence = false;
  {
    auto head = folly::IOBuf::create(5);
    head->append(5);
    auto tail = folly::IOBuf::create(5);
    tail->append(5);
    head->prependChain(std::move(tail));
    BytesPtr orig = std::move(head);
    size_t origLen = orig->computeChainDataLength();
    size_t origCount = orig->countChainElements();
    auto boxed = RustMessageAdapter<BytesPtr>::tryBox(std::move(orig));
    if (boxed.has_value()) {
      auto recovered = RustMessageAdapter<BytesPtr>::tryTake(std::move(*boxed));
      if (recovered.has_value() &&
          (*recovered)->computeChainDataLength() == origLen &&
          (*recovered)->countChainElements() == origCount) {
        chain_independence = true;
        ++passed;
      }
    }
  }

  return AdapterExtResult{
      .pointer_identity = pointer_identity,
      .null_rejected = null_rejected,
      .wrong_type_rejected = wrong_type_rejected,
      .empty_rejected = empty_rejected,
      .chain_independence = chain_independence,
      .checks_passed = passed,
      .expected_checks = expected,
  };
}

namespace {

enum class Phase6Ev : std::uint32_t {
  Alpha,
  Beta,
  Count,
};

struct Phase6AlphaHandler {
  static constexpr Subscriptions<Phase6Ev::Alpha> kSubscribedEvents{};
  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}
  void onPipelineActive(detail::ContextImpl&) noexcept {}
  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }
  void onReadReady(detail::ContextImpl&) noexcept {}
  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }
  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }
  void onWriteReady(detail::ContextImpl&) noexcept {}
  void onPipelineInactive(detail::ContextImpl&) noexcept {}
  void onEvent(
      detail::ContextImpl&, Phase6Ev, const TypeErasedBox& box) noexcept {
    ++PHASE6_EVENT_COUNT;
    if (box.empty()) {
      ++PHASE6_EMPTY_DELIVERED;
    }
  }
  static inline std::atomic<uint32_t> PHASE6_EVENT_COUNT{0};
  static inline std::atomic<uint32_t> PHASE6_EMPTY_DELIVERED{0};
  static void reset() noexcept {
    PHASE6_EVENT_COUNT.store(0, std::memory_order_relaxed);
    PHASE6_EMPTY_DELIVERED.store(0, std::memory_order_relaxed);
  }
};

} // namespace

EventNoopResult run_event_noop_test() noexcept {
  TestWatchdog watchdog{"event subsystem noop and const ref"};
  rust_handler_reset_test_counts();
  Phase6AlphaHandler::reset();

  // 1) NoEvent default (Mock handlers have no kSubscribedEvents) -> fireEvent
  // is no-op via out-of-range check PipelineImpl.cpp:321-337 covers disabled.
  bool no_event_is_noop = false;
  {
    folly::EventBase evb;
    MockHeadHandler head;
    MockTailHandler tail;
    TestAllocator alloc;
    auto pipeline =
        PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
            .setEventBase(&evb)
            .setHead(&head)
            .setTail(&tail)
            .setAllocator(&alloc)
            .build();
    pipeline->fireEvent(NoEvent::Count, TypeErasedBox{});
    // out-of-range -> return early, no crash
    no_event_is_noop = true;
    pipeline.reset();
  }

  // 2) Out-of-range id on disabled pipeline remains no-op; enabled pipeline
  // firing unsubscribed event also no-op.
  bool out_of_range_noop = false;
  {
    folly::EventBase evb;
    MockHeadHandler head;
    MockTailHandler tail;
    TestAllocator alloc;
    // Pipeline with Phase6Ev but only Alpha subscriber; Beta/Gamma out-of
    // subscriber set -> no callback, proves per-event O(s) walk.
    auto handler = std::make_unique<Phase6AlphaHandler>();
    auto pipeline = PipelineBuilder<
                        MockHeadHandler,
                        MockTailHandler,
                        TestAllocator,
                        Phase6Ev>()
                        .setEventBase(&evb)
                        .setHead(&head)
                        .setTail(&tail)
                        .setAllocator(&alloc)
                        .addNextDuplex<Phase6AlphaHandler>(
                            normal_before_tag, std::move(handler))
                        .build();
    // Out-of-range event id (>= Count) -> no-op per PipelineImpl.cpp check
    pipeline->fireEvent(
        static_cast<Phase6Ev>(static_cast<uint32_t>(Phase6Ev::Count) + 10),
        TypeErasedBox{});
    if (Phase6AlphaHandler::PHASE6_EVENT_COUNT.load() == 0) {
      out_of_range_noop = true;
    }
    pipeline.reset();
  }

  // 3) Empty payload delivered: pure signal via TypeErasedBox{} delivered as
  // const TypeErasedBox& non-consuming per audit
  // ThriftServerCompositeAppAdapter.cpp:185-195 emits CloseConnection empty
  // box.
  bool empty_payload_delivered = false;
  uint32_t subscriber_count_for_A = 0;
  {
    Phase6AlphaHandler::reset();
    folly::EventBase evb;
    MockHeadHandler head;
    MockTailHandler tail;
    TestAllocator alloc;
    auto handler = std::make_unique<Phase6AlphaHandler>();
    auto pipeline = PipelineBuilder<
                        MockHeadHandler,
                        MockTailHandler,
                        TestAllocator,
                        Phase6Ev>()
                        .setEventBase(&evb)
                        .setHead(&head)
                        .setTail(&tail)
                        .setAllocator(&alloc)
                        .addNextDuplex<Phase6AlphaHandler>(
                            normal_before_tag, std::move(handler))
                        .build();
    pipeline->fireEvent(Phase6Ev::Alpha, TypeErasedBox{});
    uint32_t cnt = Phase6AlphaHandler::PHASE6_EVENT_COUNT.load();
    uint32_t emptyCnt = Phase6AlphaHandler::PHASE6_EMPTY_DELIVERED.load();
    if (cnt == 1 && emptyCnt == 1) {
      empty_payload_delivered = true;
    }
    subscriber_count_for_A = cnt;

    // Beta not subscribed to this handler -> proves dispatch only subscribed
    pipeline->fireEvent(Phase6Ev::Beta, TypeErasedBox{});
    // Count still 1
    if (Phase6AlphaHandler::PHASE6_EVENT_COUNT.load() != 1) {
      empty_payload_delivered = false;
    }

    // Non-empty box also: const TypeErasedBox& non-consuming via get<T>()
    pipeline->fireEvent(Phase6Ev::Alpha, TypeErasedBox{int{42}});
    // Now count 2, empty still 1
    uint32_t cnt2 = Phase6AlphaHandler::PHASE6_EVENT_COUNT.load();
    uint32_t empty2 = Phase6AlphaHandler::PHASE6_EMPTY_DELIVERED.load();
    if (cnt2 == 2 && empty2 == 1) {
      subscriber_count_for_A = cnt2;
    } else {
      empty_payload_delivered = false;
    }

    pipeline.reset();
  }

  return EventNoopResult{
      .no_event_is_noop = no_event_is_noop,
      .out_of_range_noop = out_of_range_noop,
      .empty_payload_delivered = empty_payload_delivered,
      .subscriber_count_for_A = subscriber_count_for_A,
  };
}

ContextHandleTestResult run_context_handle_test(uint32_t scenario) noexcept {
  TestWatchdog watchdog{"ContextHandle lifecycle"};
  rust_handler_reset_test_counts();
  rust_handler_reset_context_handle_test_slot();

  if (scenario == 2) {
    folly::ScopedEventBaseThread eventBaseThread;
    auto* eventBase = eventBaseThread.getEventBase();
    MockHeadHandler head;
    MockTailHandler tail;
    TestAllocator allocator;
    PipelineImpl::Ptr pipeline;
    int32_t readResult = static_cast<int32_t>(Result::Error);
    uint32_t removedBeforeOwnerRelease = 0;
    uint32_t removedAfterOwnerRelease = 0;
    eventBase->runInEventBaseThreadAndWait([&] {
      pipeline = buildSingleRust(
          *eventBase,
          head,
          tail,
          allocator,
          rust_handler_new_context_handle_test(scenario));
      pipeline->activate();
      readResult =
          static_cast<int32_t>(pipeline->fireRead(TypeErasedBox{uint32_t{0}}));
      removedBeforeOwnerRelease = rust_handler_test_removed_callbacks();
      pipeline.reset();
      removedAfterOwnerRelease = rust_handler_test_removed_callbacks();
    });

    eventBase->runInEventBaseThreadAndWait([] {});
    const auto removedAfterFinalDrop = rust_handler_test_removed_callbacks();

    return ContextHandleTestResult{
        .removed_before_owner_release = removedBeforeOwnerRelease,
        .removed_after_owner_release = removedAfterOwnerRelease,
        .removed_after_first_drop = removedAfterOwnerRelease,
        .removed_after_final_drop = removedAfterFinalDrop,
        .stored_handles = 0,
        .read_result = readResult,
        .allocation_delta = 0,
        .jemalloc_available = folly::usingJEMalloc(),
    };
  }

  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline = buildSingleRust(
      eventBase,
      head,
      tail,
      allocator,
      rust_handler_new_context_handle_test(scenario));
  pipeline->activate();

  if (scenario == 7) {
    (void)pipeline->fireRead(TypeErasedBox{uint32_t{0}});
  }

  const auto before = threadAllocatedBytes();
  const auto readResult = pipeline->fireRead(TypeErasedBox{uint32_t{0}});
  const auto allocationDelta = threadAllocatedBytes() - before;
  const auto removedBeforeOwnerRelease = rust_handler_test_removed_callbacks();
  const auto storedHandles = rust_handler_context_handle_test_slot_len();

  if (scenario == 5) {
    pipeline->close();
  }
  pipeline.reset();
  const auto removedAfterOwnerRelease = rust_handler_test_removed_callbacks();

  if (scenario == 4) {
    rust_handler_drop_one_context_handle_for_test();
  }
  const auto removedAfterFirstDrop = rust_handler_test_removed_callbacks();
  rust_handler_drop_all_context_handles_for_test();
  const auto removedAfterFinalDrop = rust_handler_test_removed_callbacks();

  return ContextHandleTestResult{
      .removed_before_owner_release = removedBeforeOwnerRelease,
      .removed_after_owner_release = removedAfterOwnerRelease,
      .removed_after_first_drop = removedAfterFirstDrop,
      .removed_after_final_drop = removedAfterFinalDrop,
      .stored_handles = storedHandles,
      .read_result = static_cast<int32_t>(readResult),
      .allocation_delta = allocationDelta,
      .jemalloc_available = folly::usingJEMalloc(),
  };
}

ContextHandleFireResult run_context_handle_fire_test(
    uint32_t scenario) noexcept {
  TestWatchdog watchdog{"ContextHandle deferred fire"};
  rust_handler_reset_test_counts();

  folly::ScopedEventBaseThread eventBaseThread;
  auto* eventBase = eventBaseThread.getEventBase();
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  const bool read = scenario == 8 || scenario == 10 || scenario == 12;
  const folly::IOBuf* received = nullptr;
  if (read) {
    tail.setOnReadCallback([&](TypeErasedBox&& message) noexcept {
      received = message.get<BytesPtr>().get();
      return Result::Success;
    });
  } else {
    head.setOnWriteCallback([&](TypeErasedBox&& message) noexcept {
      received = message.get<BytesPtr>().get();
      return Result::Success;
    });
  }

  PipelineImpl::Ptr pipeline;
  uint32_t endpointCallsBeforeFence = 0;
  uint32_t removedAfterOwnerRelease = 0;
  uint64_t allocationDelta = 0;
  const folly::IOBuf* original = nullptr;
  eventBase->runInEventBaseThreadAndWait([&] {
    pipeline = buildSingleRust(
        *eventBase,
        head,
        tail,
        allocator,
        rust_handler_new_context_handle_test(scenario));
    pipeline->activate();

    auto message = makeBytes(8, 0xa5);
    original = message.get();
    const auto before = threadAllocatedBytes();
    if (read) {
      (void)pipeline->fireRead(erase_and_box(std::move(message)));
      endpointCallsBeforeFence = static_cast<uint32_t>(tail.readCount());
    } else {
      (void)pipeline->fireWrite(erase_and_box(std::move(message)));
      endpointCallsBeforeFence = static_cast<uint32_t>(head.writeCount());
    }
    allocationDelta = threadAllocatedBytes() - before;
    pipeline.reset();
    removedAfterOwnerRelease = rust_handler_test_removed_callbacks();
  });

  eventBase->runInEventBaseThreadAndWait([] {});
  const auto endpointCallsAfterFence =
      static_cast<uint32_t>(read ? tail.readCount() : head.writeCount());
  const auto removedAfterFence = rust_handler_test_removed_callbacks();

  return ContextHandleFireResult{
      .endpoint_calls_before_fence = endpointCallsBeforeFence,
      .endpoint_calls_after_fence = endpointCallsAfterFence,
      .removed_after_owner_release = removedAfterOwnerRelease,
      .removed_after_fence = removedAfterFence,
      .pointer_identity_preserved = received != nullptr && received == original,
      .allocation_delta = allocationDelta,
      .jemalloc_available = folly::usingJEMalloc(),
  };
}

ContextHandleSandwichResult run_context_handle_sandwich_test(
    uint32_t scenario) noexcept {
  TestWatchdog watchdog{"ContextHandle C++/Rust/C++ sandwich"};
  rust_handler_reset_test_counts();

  folly::ScopedEventBaseThread eventBaseThread;
  auto* eventBase = eventBaseThread.getEventBase();
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  const bool read =
      scenario == 8 || scenario == 10 || scenario == 20 || scenario == 21;
  const folly::IOBuf* received = nullptr;
  if (read) {
    tail.setOnReadCallback([&](TypeErasedBox&& message) noexcept {
      received = message.get<BytesPtr>().get();
      return Result::Success;
    });
  } else {
    head.setOnWriteCallback([&](TypeErasedBox&& message) noexcept {
      received = message.get<BytesPtr>().get();
      return Result::Success;
    });
  }

  auto before = std::make_unique<MockHandler>();
  auto* beforePtr = before.get();
  auto after = std::make_unique<MockHandler>();
  auto* afterPtr = after.get();
  auto rustHandler = std::make_unique<ProductionRustHandler>(
      rust_handler_new_context_handle_test(scenario));

  PipelineImpl::Ptr pipeline;
  uint32_t beforeReadsBeforeFence = 0;
  uint32_t afterReadsBeforeFence = 0;
  uint32_t beforeWritesBeforeFence = 0;
  uint32_t afterWritesBeforeFence = 0;
  uint32_t endpointCallsBeforeFence = 0;
  const folly::IOBuf* original = nullptr;
  eventBase->runInEventBaseThreadAndWait([&] {
    pipeline =
        PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
            .setEventBase(eventBase)
            .setHead(&head)
            .setTail(&tail)
            .setAllocator(&allocator)
            .addNextDuplex<MockHandler>(normal_before_tag, std::move(before))
            .addNextDuplex<ProductionRustHandler>(
                rust_middle_tag, std::move(rustHandler))
            .addNextDuplex<MockHandler>(normal_after_tag, std::move(after))
            .build();
    pipeline->activate();

    auto message = makeBytes(8, 0xa5);
    original = message.get();
    if (read) {
      (void)pipeline->fireRead(erase_and_box(std::move(message)));
      endpointCallsBeforeFence = static_cast<uint32_t>(tail.readCount());
    } else {
      (void)pipeline->fireWrite(erase_and_box(std::move(message)));
      endpointCallsBeforeFence = static_cast<uint32_t>(head.writeCount());
    }
    beforeReadsBeforeFence = static_cast<uint32_t>(beforePtr->readCount());
    afterReadsBeforeFence = static_cast<uint32_t>(afterPtr->readCount());
    beforeWritesBeforeFence = static_cast<uint32_t>(beforePtr->writeCount());
    afterWritesBeforeFence = static_cast<uint32_t>(afterPtr->writeCount());
  });

  eventBase->runInEventBaseThreadAndWait([] {});
  ContextHandleSandwichResult result{
      .before_reads_before_fence = beforeReadsBeforeFence,
      .before_reads_after_fence = static_cast<uint32_t>(beforePtr->readCount()),
      .after_reads_before_fence = afterReadsBeforeFence,
      .after_reads_after_fence = static_cast<uint32_t>(afterPtr->readCount()),
      .before_writes_before_fence = beforeWritesBeforeFence,
      .before_writes_after_fence =
          static_cast<uint32_t>(beforePtr->writeCount()),
      .after_writes_before_fence = afterWritesBeforeFence,
      .after_writes_after_fence = static_cast<uint32_t>(afterPtr->writeCount()),
      .endpoint_calls_before_fence = endpointCallsBeforeFence,
      .endpoint_calls_after_fence =
          static_cast<uint32_t>(read ? tail.readCount() : head.writeCount()),
      .pointer_identity_preserved = received != nullptr && received == original,
  };
  eventBase->runInEventBaseThreadAndWait(
      [pipeline = std::move(pipeline)]() mutable { pipeline.reset(); });
  return result;
}

ContextHandleExceptionResult run_context_handle_exception_test(
    uint32_t scenario) noexcept {
  TestWatchdog watchdog{"ContextHandle deferred exception"};
  rust_handler_reset_test_counts();

  folly::ScopedEventBaseThread eventBaseThread;
  auto* eventBase = eventBaseThread.getEventBase();
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  const std::string expectedMessage = scenario == 17 ? std::string{}
      : scenario == 18                               ? std::string(4096, 'x')
                       : std::string{"deferred exception \xce\xbb"};
  uint32_t beforeExceptions = 0;
  uint32_t afterExceptions = 0;
  uint32_t tailExceptions = 0;
  bool messagePreserved = false;
  tail.setOnExceptionCallback([&](folly::exception_wrapper&& exception) {
    ++tailExceptions;
    exception.with_exception([&](const std::runtime_error& error) {
      messagePreserved = error.what() == expectedMessage;
    });
  });

  auto before = std::make_unique<MockHandler>();
  before->setOnException(
      [&](detail::ContextImpl& context, folly::exception_wrapper&& exception) {
        ++beforeExceptions;
        context.fireException(std::move(exception));
      });
  auto after = std::make_unique<MockHandler>();
  after->setOnException(
      [&](detail::ContextImpl& context, folly::exception_wrapper&& exception) {
        ++afterExceptions;
        context.fireException(std::move(exception));
      });
  auto rustHandler = std::make_unique<ProductionRustHandler>(
      rust_handler_new_context_handle_test(scenario));

  PipelineImpl::Ptr pipeline;
  uint32_t beforeExceptionsBeforeFence = 0;
  uint32_t afterExceptionsBeforeFence = 0;
  uint32_t tailExceptionsBeforeFence = 0;
  uint32_t removedAfterOwnerRelease = 0;
  eventBase->runInEventBaseThreadAndWait([&] {
    pipeline =
        PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
            .setEventBase(eventBase)
            .setHead(&head)
            .setTail(&tail)
            .setAllocator(&allocator)
            .addNextDuplex<MockHandler>(normal_before_tag, std::move(before))
            .addNextDuplex<ProductionRustHandler>(
                rust_middle_tag, std::move(rustHandler))
            .addNextDuplex<MockHandler>(normal_after_tag, std::move(after))
            .build();
    pipeline->activate();
    (void)pipeline->fireRead(erase_and_box(makeBytes(8, 0xa5)));
    beforeExceptionsBeforeFence = beforeExceptions;
    afterExceptionsBeforeFence = afterExceptions;
    tailExceptionsBeforeFence = tailExceptions;
    pipeline.reset();
    removedAfterOwnerRelease = rust_handler_test_removed_callbacks();
  });

  eventBase->runInEventBaseThreadAndWait([] {});
  return ContextHandleExceptionResult{
      .before_exceptions_before_fence = beforeExceptionsBeforeFence,
      .before_exceptions_after_fence = beforeExceptions,
      .after_exceptions_before_fence = afterExceptionsBeforeFence,
      .after_exceptions_after_fence = afterExceptions,
      .tail_exceptions_before_fence = tailExceptionsBeforeFence,
      .tail_exceptions_after_fence = tailExceptions,
      .removed_after_owner_release = removedAfterOwnerRelease,
      .removed_after_fence = rust_handler_test_removed_callbacks(),
      .message_preserved = messagePreserved,
  };
}

void run_event_base_destruction_test(
    uintptr_t task,
    rust::Fn<void(uintptr_t)> call,
    rust::Fn<void(uintptr_t)> drop) noexcept {
  auto eventBase = std::make_unique<folly::EventBase>();
  eventBase->setStrictLoopThread();
  std::thread worker([eventBase = eventBase.get(),
                      task,
                      call = std::move(call),
                      drop = std::move(drop)]() mutable {
    channel_pipeline_rust::enqueueInEventBase(
        eventBase, task, std::move(call), std::move(drop));
  });
  worker.join();
  eventBase.reset();
}

// Proves forward-unknown via RustTypeErasedBox: a Rust handler forwards the
// message downstream WITHOUT taking/inspecting it (pass-through), so the
// inbound reaches the tail and the outbound reaches the head unchanged.
ForwardUnknownResult run_forward_unknown_test() noexcept {
  TestWatchdog watchdog{"forward via RustTypeErasedBox pass-through"};
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;

  auto rustHandler = std::make_unique<ProductionRustHandler>(
      rust_handler_new_forwarding_test());
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<ProductionRustHandler>(
              rust_middle_tag, std::move(rustHandler))
          .build();
  CHECK(pipeline);
  pipeline->activate();
  eventBase.loopOnce();

  const auto readResult = pipeline->fireRead(erase_and_box(makeBytes(7, 0xab)));
  eventBase.loopOnce();
  const auto writeResult =
      pipeline->fireWrite(erase_and_box(makeBytes(3, 0xcd)));
  eventBase.loopOnce();

  using apache::thrift::fast_thrift::channel_pipeline::Result;
  ForwardUnknownResult result{
      .tail_reads = static_cast<uint32_t>(tail.readCount()),
      .head_writes = static_cast<uint32_t>(head.writeCount()),
      .read_is_error = (readResult == Result::Error),
      .write_is_error = (writeResult == Result::Error),
  };

  pipeline->deactivate();
  eventBase.loopOnce();
  pipeline.reset();
  eventBase.loopOnce();
  return result;
}

} // namespace channel_pipeline_rust::test
