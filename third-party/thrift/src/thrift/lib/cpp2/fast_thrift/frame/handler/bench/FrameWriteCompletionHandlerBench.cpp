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

/**
 * FrameWriteCompletionHandler cost microbenchmark.
 *
 * Measures the per-frame write-completion tracking overhead against a
 * no-tracking baseline. Both pipelines run the same serializer + batcher on the
 * outbound path, so the BENCHMARK_RELATIVE number isolates exactly what the
 * handler adds: the transport per-writev event, the batcher's frame-count FIFO,
 * and the handler's per-frame streamId FIFO plus the per-frame
 * FrameWriteComplete fan-out to a real subscriber.
 *
 *   baseline: TransportHandler (NoOp factory)
 *             -> LoopBatchingFrameHandler (NoOp tracker)
 *             -> BenchSerializer  (ComposedFrame -> IOBuf)
 *             -> tail                                   (events disabled)
 *
 *   handler:  TransportHandlerT<RocketClientEventFactory>
 *             -> LoopBatchingFrameHandlerT<WriteCompletionTrackerT<...>>
 *             -> BenchSerializer
 *             -> BenchWriteCompleteSubscriber (subscribes FrameWriteComplete)
 *             -> FrameWriteCompletionHandlerT<RocketClientEventFactory>
 *                (subscribes BatchWriteComplete, fans out per-frame
 *                FrameWriteComplete)
 *             -> tail
 *
 * Tracking only engages on write completion, so the benchmarks drive the
 * request (outbound) path; BenchAsyncTransport fires writeSuccess inline.
 */

#include <cstddef>
#include <cstdint>
#include <vector>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/handler/FrameWriteCompletionHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/LoopBatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/WriteCompletionTracker.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/RocketClientEventFactory.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>
#include <thrift/lib/cpp2/fast_thrift/transport/bench/BenchAsyncTransport.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame::handler;
using namespace apache::thrift::fast_thrift::frame::write::handler;
using namespace apache::thrift::fast_thrift::rocket::client;
using namespace apache::thrift::fast_thrift::transport::bench;

namespace {

constexpr size_t kBytesPerFrame = 256;

// Frames coalesced into a single batch (one write-completion per batch).
constexpr size_t kSmallBatch = 4;
constexpr size_t kMediumBatch = 32;
constexpr size_t kLargeBatch = 256;

HANDLER_TAG(batching);
HANDLER_TAG(serializer);
HANDLER_TAG(subscriber);
HANDLER_TAG(handler);

// Layer-boundary stand-in for the rocket codec: consumes the ComposedFrame
// (mirroring real serialization) and emits a fixed-size IOBuf for the batcher.
// Present in both pipelines so the relative benchmark isolates only the
// tracking machinery.
class BenchSerializer {
 public:
  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  template <typename Context>
  Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  Result onWrite(Context& ctx, TypeErasedBox&& msg) noexcept {
    (void)msg.take<frame::ComposedFrame>();
    auto buf = folly::IOBuf::create(kBytesPerFrame);
    buf->append(kBytesPerFrame);
    return ctx.fireWrite(TypeErasedBox(std::move(buf)));
  }
};

// Real per-frame consumer: subscribes to the handler's FrameWriteComplete and
// touches the carried streamId so the fan-out can't be optimized away.
class BenchWriteCompleteSubscriber {
 public:
  using EventId = RocketClientEventId;
  static constexpr apache::thrift::fast_thrift::channel_pipeline::Subscriptions<
      EventId::FrameWriteComplete>
      kSubscribedEvents{};

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }
  template <typename Context>
  Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }
  template <typename Context>
  Result onWrite(Context& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onEvent(
      Context& /*ctx*/, EventId /*ev*/, const TypeErasedBox& box) noexcept {
    folly::doNotOptimizeAway(box.get<FrameWriteCompleteEvent>().streamId);
  }
};

// Fixture parameterized on the transport's event factory, the batcher's
// tracker, the pipeline event enum, and whether the write-completion handler is
// wired in.
template <
    typename Factory,
    typename Tracker,
    typename EventEnumT,
    bool WithHandler>
struct FixtureT {
  static constexpr bool kWithHandler = WithHandler;

  using TransportHandler =
      apache::thrift::fast_thrift::transport::TransportHandlerT<Factory>;
  using Batcher = LoopBatchingFrameHandlerT<Tracker>;

  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  RocketClientAppAdapter::Ptr appAdapter{new RocketClientAppAdapter()};
  typename TransportHandler::Ptr transportHandler;
  PipelineImpl::Ptr pipeline;
  SimpleBufferAllocator allocator;

  FixtureT() = default;
  FixtureT(const FixtureT&) = delete;
  FixtureT& operator=(const FixtureT&) = delete;
  FixtureT(FixtureT&&) = delete;
  FixtureT& operator=(FixtureT&&) = delete;
  ~FixtureT() {
    if (transportHandler) {
      transportHandler->close(folly::exception_wrapper{});
      transportHandler->resetPipeline();
    }
    pipeline.reset();
  }

  void setup() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(&evb));
    testTransport = static_cast<BenchAsyncTransport*>(transport.get());
    transportHandler = TransportHandler::create(std::move(transport));

    // PipelineBuilder is move/copy-deleted, so the whole chain must be a single
    // expression — branch on whether the handler is wired in.
    if constexpr (WithHandler) {
      pipeline =
          PipelineBuilder<
              TransportHandler,
              RocketClientAppAdapter,
              SimpleBufferAllocator,
              EventEnumT>()
              .setEventBase(&evb)
              .setHead(transportHandler.get())
              .setTail(appAdapter.get())
              .setAllocator(&allocator)
              .template addNextOutbound<Batcher>(batching_tag)
              .template addNextDuplex<BenchSerializer>(serializer_tag)
              .template addNextDuplex<BenchWriteCompleteSubscriber>(
                  subscriber_tag)
              .template addNextDuplex<
                  FrameWriteCompletionHandlerT<RocketClientEventFactory>>(
                  handler_tag)
              .build();
    } else {
      pipeline = PipelineBuilder<
                     TransportHandler,
                     RocketClientAppAdapter,
                     SimpleBufferAllocator,
                     EventEnumT>()
                     .setEventBase(&evb)
                     .setHead(transportHandler.get())
                     .setTail(appAdapter.get())
                     .setAllocator(&allocator)
                     .template addNextOutbound<Batcher>(batching_tag)
                     .template addNextDuplex<BenchSerializer>(serializer_tag)
                     .build();
    }

    transportHandler->setPipeline(pipeline.get());
    transportHandler->onConnect();
  }
};

using BaselineFixture = FixtureT<
    apache::thrift::fast_thrift::transport::NoOpWriteCompleteEventFactory,
    NoOpWriteCompletionTracker,
    NoEvent,
    /*WithHandler=*/false>;
using HandlerFixture = FixtureT<
    RocketClientEventFactory,
    WriteCompletionTrackerT<RocketClientEventFactory>,
    RocketClientEventId,
    /*WithHandler=*/true>;

// Drives `iters` batches of `framesPerBatch` outbound ComposedFrames. Frames
// are preallocated outside the timed region and carry distinct streamIds (as
// the upstream stream-state layer would have assigned). Each batch is flushed
// by two loop iterations (LoopBatchingFrameHandler reschedules once before
// flushing); BenchAsyncTransport fires writeSuccess inline from writeChain, so
// the write-completion path runs synchronously.
template <typename Fixture>
void runWriteBench(size_t iters, size_t framesPerBatch) {
  folly::BenchmarkSuspender suspender;
  Fixture fixture;
  fixture.setup();

  const size_t total = iters * framesPerBatch;
  std::vector<frame::ComposedFrame> frames;
  frames.reserve(total);
  for (size_t i = 0; i < total; ++i) {
    frames.push_back(
        frame::ComposedFrame{
            .frameType = frame::FrameType::REQUEST_RESPONSE,
            .streamId = static_cast<uint32_t>(2 * i + 1),
        });
  }

  suspender.dismiss();

  auto frameIt = frames.begin();
  for (size_t i = 0; i < iters; ++i) {
    for (size_t j = 0; j < framesPerBatch; ++j) {
      (void)fixture.pipeline->fireWrite(erase_and_box(std::move(*frameIt++)));
    }
    fixture.evb.loopOnce(EVLOOP_NONBLOCK);
    fixture.evb.loopOnce(EVLOOP_NONBLOCK);
  }
}

BENCHMARK(NoTracking_SmallBatch, iters) {
  runWriteBench<BaselineFixture>(iters, kSmallBatch);
}
BENCHMARK_RELATIVE(WriteCompletionHandler_SmallBatch, iters) {
  runWriteBench<HandlerFixture>(iters, kSmallBatch);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(NoTracking_MediumBatch, iters) {
  runWriteBench<BaselineFixture>(iters, kMediumBatch);
}
BENCHMARK_RELATIVE(WriteCompletionHandler_MediumBatch, iters) {
  runWriteBench<HandlerFixture>(iters, kMediumBatch);
}

BENCHMARK_DRAW_LINE();

BENCHMARK(NoTracking_LargeBatch, iters) {
  runWriteBench<BaselineFixture>(iters, kLargeBatch);
}
BENCHMARK_RELATIVE(WriteCompletionHandler_LargeBatch, iters) {
  runWriteBench<HandlerFixture>(iters, kLargeBatch);
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
