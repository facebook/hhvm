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
 * ThriftServerCompositeAppAdapter routing microbenchmark.
 *
 * Isolates the per-request cost the composite adds over a bare adapter:
 * the F14 methodMap probe + the extra onRead hop into the chosen child.
 * The child's per-method thunk is a no-op that returns Success without
 * touching the pipeline, so each iter measures only the routing decision
 * (no rocket framing, no protocol parsing, no wire I/O).
 *
 * Comparisons:
 *   - BareAdapter_*           : dispatch straight into a ThriftServerAppAdapter
 *                               (no composite). Establishes the per-request
 *                               floor for the adapter machinery itself.
 *   - Composite_OneChild_*    : composite wrapping a single child. Measures
 *                               the additive cost of the composite layer when
 *                               there is no actual fan-out — i.e. the price
 *                               you pay by enabling composite at all.
 *   - Composite_NChildren_*   : composite over 2 / 4 children, hitting the
 *                               first vs the last child. F14 is O(1), so
 *                               first/last should be flat — these are
 *                               regression guards in case methodMap ever
 *                               grows non-trivial.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftRequestPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <string>
#include <vector>

namespace {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::thrift::ThriftRequestResponsePayload;
using apache::thrift::fast_thrift::thrift::ThriftServerAppAdapter;
using apache::thrift::fast_thrift::thrift::ThriftServerCompositeAppAdapter;
using apache::thrift::fast_thrift::thrift::ThriftServerInboundPayloadVariant;
using apache::thrift::fast_thrift::thrift::ThriftServerRequestMessage;

// Adapter whose registered methods are no-op thunks. Lets us bench routing
// in isolation: addMethodHandler inserts into dispatch_; the thunk returns
// Success without writing to a pipeline (so no pipeline wiring is needed).
class NoOpAdapter : public ThriftServerAppAdapter {
 public:
  using Ptr = std::unique_ptr<NoOpAdapter, Destructor>;

  void registerMethod(std::string_view name) {
    addMethodHandler(
        name,
        +[](ThriftServerAppAdapter* /*self*/,
            uint32_t /*streamId*/,
            std::unique_ptr<folly::IOBuf> /*data*/,
            apache::thrift::ProtocolId /*protocol*/) noexcept -> Result {
          return Result::Success;
        });
  }
};

ThriftServerRequestMessage makeRequest(
    std::string_view methodName, uint32_t streamId) {
  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  metadata->name().emplace(methodName);
  metadata->kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata->protocol() = apache::thrift::ProtocolId::BINARY;

  ThriftServerRequestMessage msg;
  msg.payload = ThriftServerInboundPayloadVariant{ThriftRequestResponsePayload{
      .data = folly::IOBuf::copyBuffer("payload"),
      .metadata = std::move(metadata)}};
  msg.streamId = streamId;
  return msg;
}

// Build N children, each owning four distinct methods. Returns the composite
// plus the method names registered for child indices the bench wants to hit.
struct CompositeFixture {
  ThriftServerCompositeAppAdapter::Ptr composite;
  std::vector<std::string> methodNames; // one method name per child
};

CompositeFixture makeComposite(size_t numChildren) {
  CompositeFixture fixture;
  fixture.composite = ThriftServerCompositeAppAdapter::Ptr{
      new ThriftServerCompositeAppAdapter()};
  fixture.methodNames.reserve(numChildren);

  for (size_t i = 0; i < numChildren; ++i) {
    NoOpAdapter::Ptr child{new NoOpAdapter()};
    // Pad each child with extra methods so methodMap_ holds 4 entries per
    // child — closer to a realistic service shape than 1.
    for (int m = 0; m < 4; ++m) {
      child->registerMethod(
          "child" + std::to_string(i) + "_method" + std::to_string(m));
    }
    fixture.methodNames.push_back("child" + std::to_string(i) + "_method0");
    fixture.composite->addChild(ThriftServerAppAdapter::Ptr{child.release()});
  }
  return fixture;
}

NoOpAdapter::Ptr makeBareAdapter() {
  NoOpAdapter::Ptr adapter{new NoOpAdapter()};
  for (int m = 0; m < 4; ++m) {
    adapter->registerMethod("bare_method" + std::to_string(m));
  }
  return adapter;
}

// Pre-build `iters` request messages for `methodName`. Each iter consumes
// one (move-into-box semantics) so the loop body stays allocation-free.
std::vector<ThriftServerRequestMessage> prebuildRequests(
    size_t iters, std::string_view methodName) {
  std::vector<ThriftServerRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    // streamId only needs to be non-zero — composite DCHECKs it but doesn't
    // act on the value otherwise.
    requests.push_back(makeRequest(methodName, /*streamId=*/1));
  }
  return requests;
}

// =============================================================================
// Baseline: dispatch directly into a bare adapter (no composite layer).
// =============================================================================

BENCHMARK(BareAdapter_HitMethod, iters) {
  folly::BenchmarkSuspender suspender;
  auto adapter = makeBareAdapter();
  auto requests = prebuildRequests(iters, "bare_method0");
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = adapter->onRead(erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

// =============================================================================
// Composite-of-one: measures composite layer overhead with no fan-out.
// =============================================================================

BENCHMARK_RELATIVE(Composite_OneChild_HitMethod, iters) {
  folly::BenchmarkSuspender suspender;
  auto fixture = makeComposite(/*numChildren=*/1);
  auto requests = prebuildRequests(iters, fixture.methodNames[0]);
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result =
        fixture.composite->onRead(erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Two children: hitting first vs last child should be flat (F14 is O(1)).
// =============================================================================

BENCHMARK(Composite_TwoChildren_HitFirst, iters) {
  folly::BenchmarkSuspender suspender;
  auto fixture = makeComposite(/*numChildren=*/2);
  auto requests = prebuildRequests(iters, fixture.methodNames[0]);
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result =
        fixture.composite->onRead(erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(Composite_TwoChildren_HitLast, iters) {
  folly::BenchmarkSuspender suspender;
  auto fixture = makeComposite(/*numChildren=*/2);
  auto requests = prebuildRequests(iters, fixture.methodNames[1]);
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result =
        fixture.composite->onRead(erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Four children: same first/last comparison at higher methodMap fill.
// =============================================================================

BENCHMARK(Composite_FourChildren_HitFirst, iters) {
  folly::BenchmarkSuspender suspender;
  auto fixture = makeComposite(/*numChildren=*/4);
  auto requests = prebuildRequests(iters, fixture.methodNames[0]);
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result =
        fixture.composite->onRead(erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(Composite_FourChildren_HitLast, iters) {
  folly::BenchmarkSuspender suspender;
  auto fixture = makeComposite(/*numChildren=*/4);
  auto requests = prebuildRequests(iters, fixture.methodNames[3]);
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result =
        fixture.composite->onRead(erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
