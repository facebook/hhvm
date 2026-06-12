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
 * RocketClient Write-Completion Tracking Microbenchmarks
 *
 * Isolates the cost of per-write completion tracking on the outbound path.
 * Two pipelines, identical except for the tracking machinery, are compared:
 *
 *   baseline: TransportHandler (NoOp factory)
 *             → LoopBatchingFrameHandler (NoOp tracker)
 *             → tail                              (events disabled)
 *
 *   tracking: TransportHandlerT<RocketClientEventFactory>
 *             → LoopBatchingFrameHandlerT<WriteCompletionTrackerT<...>>
 *             → tail (subscribes to RocketClientEventId)
 *
 * The baseline is the BENCHMARK reference and the tracking variant is its
 * BENCHMARK_RELATIVE, so the relative number is the tracking overhead: the
 * per-write event construction, the batch frame-count FIFO, and the enriched
 * re-fire to a subscriber. Tracking only engages on write completion, so the
 * benchmarks drive the request (outbound) path.
 */

#include <cstddef>
#include <memory>
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
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/LoopBatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/WriteCompletionTracker.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/RocketClientEventFactory.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>
#include <thrift/lib/cpp2/fast_thrift/transport/bench/BenchAsyncTransport.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame::write::handler;
using namespace apache::thrift::fast_thrift::rocket::client;
using namespace apache::thrift::fast_thrift::transport::bench;

namespace {

constexpr size_t kBytesPerFrame = 256;

// Frames coalesced into a single batch (one write-completion event per batch).
// Spanning small/medium/large shows how the fixed per-completion tracking cost
// amortizes as batches grow.
constexpr size_t kSmallBatch = 4;
constexpr size_t kMediumBatch = 32;
constexpr size_t kLargeBatch = 256;

HANDLER_TAG(batching);

// Fixture parameterized on the transport's event factory, the batcher's
// tracker, and the pipeline event enum. The tail is the real
// RocketClientAppAdapter — in the tracking pipeline it subscribes to the
// enriched write-completion event and runs the owner's onWriteComplete
// callback; in the baseline pipeline (events disabled) the subscription
// compiles out. The baseline and tracking variants differ only in the three
// template parameters, so the relative benchmark isolates tracking cost
// through the real consumer path.
template <typename Factory, typename Tracker, typename EventEnumT>
struct FixtureT {
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
                   .build();

    // Real consumer callback — fires once per completed batch in the tracking
    // pipeline; never invoked in the baseline (subscription compiled out).
    appAdapter->setOnWriteComplete(
        [](const RocketWriteCompleteEvent&) noexcept {});

    transportHandler->setPipeline(pipeline.get());
    transportHandler->onConnect();
  }
};

using BaselineFixture = FixtureT<
    apache::thrift::fast_thrift::transport::NoOpWriteCompleteEventFactory,
    NoOpWriteCompletionTracker,
    NoEvent>;
using TrackingFixture = FixtureT<
    RocketClientEventFactory,
    WriteCompletionTrackerT<RocketClientEventFactory>,
    RocketClientEventId>;

// Drives `iters` batches of `framesPerBatch` outbound frames each. Frames are
// preallocated outside the timed region. Each batch is flushed by two loop
// iterations (LoopBatchingFrameHandler reschedules once before flushing);
// BenchAsyncTransport fires writeSuccess inline from writeChain, so the
// write-completion path runs synchronously within the flush.
template <typename Fixture>
void runWriteBench(size_t iters, size_t framesPerBatch) {
  folly::BenchmarkSuspender suspender;
  Fixture fixture;
  fixture.setup();

  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters * framesPerBatch);
  for (size_t i = 0; i < iters * framesPerBatch; ++i) {
    auto frame = folly::IOBuf::create(kBytesPerFrame);
    frame->append(kBytesPerFrame);
    frames.push_back(std::move(frame));
  }

  suspender.dismiss();

  size_t idx = 0;
  for (size_t i = 0; i < iters; ++i) {
    for (size_t j = 0; j < framesPerBatch; ++j) {
      (void)fixture.pipeline->fireWrite(
          TypeErasedBox(std::move(frames[idx++])));
    }
    fixture.evb.loopOnce(EVLOOP_NONBLOCK);
    fixture.evb.loopOnce(EVLOOP_NONBLOCK);
  }
}

// =============================================================================
// Small batch — fixed per-completion tracking cost is least amortized.
// =============================================================================

BENCHMARK(NoTracking_SmallBatch, iters) {
  runWriteBench<BaselineFixture>(iters, kSmallBatch);
}

BENCHMARK_RELATIVE(WriteTracking_SmallBatch, iters) {
  runWriteBench<TrackingFixture>(iters, kSmallBatch);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Medium batch.
// =============================================================================

BENCHMARK(NoTracking_MediumBatch, iters) {
  runWriteBench<BaselineFixture>(iters, kMediumBatch);
}

BENCHMARK_RELATIVE(WriteTracking_MediumBatch, iters) {
  runWriteBench<TrackingFixture>(iters, kMediumBatch);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Large batch — tracking cost amortized across many frames per completion.
// =============================================================================

BENCHMARK(NoTracking_LargeBatch, iters) {
  runWriteBench<BaselineFixture>(iters, kLargeBatch);
}

BENCHMARK_RELATIVE(WriteTracking_LargeBatch, iters) {
  runWriteBench<TrackingFixture>(iters, kLargeBatch);
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
