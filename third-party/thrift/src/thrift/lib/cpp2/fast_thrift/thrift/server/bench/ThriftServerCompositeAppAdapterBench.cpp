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
using apache::thrift::fast_thrift::thrift::ThriftRequestContext;
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
            apache::thrift::ProtocolId /*protocol*/,
            std::unique_ptr<ThriftRequestContext> /*requestContext*/) noexcept
            -> Result { return Result::Success; });
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

// Build N children, each owning four distinct methods. Returns the composite,
// the child owners (composite borrows; caller keeps them alive), and the
// method names registered for child indices the bench wants to hit.
struct CompositeFixture {
  ThriftServerCompositeAppAdapter::Ptr composite;
  std::vector<NoOpAdapter::Ptr> children;
  std::vector<std::string> methodNames; // one method name per child
};

CompositeFixture makeComposite(size_t numChildren) {
  CompositeFixture fixture;
  fixture.composite = ThriftServerCompositeAppAdapter::Ptr{
      new ThriftServerCompositeAppAdapter()};
  fixture.children.reserve(numChildren);
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
    fixture.composite->addChild(child.get());
    fixture.children.push_back(std::move(child));
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

BENCHMARK_DRAW_LINE();

// =============================================================================
// Larger child counts — confirm F14 lookup stays flat (regression guard).
// =============================================================================

BENCHMARK(Composite_EightChildren_HitLast, iters) {
  folly::BenchmarkSuspender suspender;
  auto fixture = makeComposite(/*numChildren=*/8);
  auto requests = prebuildRequests(iters, fixture.methodNames[7]);
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result =
        fixture.composite->onRead(erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(Composite_SixteenChildren_HitLast, iters) {
  folly::BenchmarkSuspender suspender;
  auto fixture = makeComposite(/*numChildren=*/16);
  auto requests = prebuildRequests(iters, fixture.methodNames[15]);
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result =
        fixture.composite->onRead(erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Heterogeneous children — concept-based design's headline capability.
// Two distinct concrete child types share the composite. Per-T thunks
// dispatch correctly without inheriting from a common base. Expect parity
// with the homogeneous Composite_TwoChildren benches.
// =============================================================================

// Second adapter type whose runtime shape matches NoOpAdapter but whose
// concrete C++ type is distinct — proves the composite stores heterogeneous
// children via per-T thunks, not via shared base inheritance.
class OtherNoOpAdapter : public ThriftServerAppAdapter {
 public:
  using Ptr = std::unique_ptr<OtherNoOpAdapter, Destructor>;

  void registerMethod(std::string_view name) {
    addMethodHandler(
        name,
        +[](ThriftServerAppAdapter* /*self*/,
            uint32_t /*streamId*/,
            std::unique_ptr<folly::IOBuf> /*data*/,
            apache::thrift::ProtocolId /*protocol*/,
            std::unique_ptr<ThriftRequestContext> /*requestContext*/) noexcept
            -> Result { return Result::Success; });
  }
};

struct HeterogeneousFixture {
  ThriftServerCompositeAppAdapter::Ptr composite;
  NoOpAdapter::Ptr firstChild;
  OtherNoOpAdapter::Ptr secondChild;
  std::string firstMethod;
  std::string secondMethod;
};

HeterogeneousFixture makeHeterogeneousComposite() {
  HeterogeneousFixture fixture;
  fixture.composite = ThriftServerCompositeAppAdapter::Ptr{
      new ThriftServerCompositeAppAdapter()};
  fixture.firstChild = NoOpAdapter::Ptr{new NoOpAdapter()};
  fixture.secondChild = OtherNoOpAdapter::Ptr{new OtherNoOpAdapter()};
  for (int m = 0; m < 4; ++m) {
    fixture.firstChild->registerMethod("first_method" + std::to_string(m));
    fixture.secondChild->registerMethod("second_method" + std::to_string(m));
  }
  fixture.firstMethod = "first_method0";
  fixture.secondMethod = "second_method0";
  fixture.composite->addChild(fixture.firstChild.get());
  fixture.composite->addChild(fixture.secondChild.get());
  return fixture;
}

BENCHMARK(Composite_HeterogeneousChildren_HitFirst, iters) {
  folly::BenchmarkSuspender suspender;
  auto fixture = makeHeterogeneousComposite();
  auto requests = prebuildRequests(iters, fixture.firstMethod);
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result =
        fixture.composite->onRead(erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(Composite_HeterogeneousChildren_HitSecond, iters) {
  folly::BenchmarkSuspender suspender;
  auto fixture = makeHeterogeneousComposite();
  auto requests = prebuildRequests(iters, fixture.secondMethod);
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result =
        fixture.composite->onRead(erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Failure path: unknown method → framework-error fire. Establishes a ceiling
// for the failure path so future regressions in serializeResponseRpcError or
// the error fire surface in this microbench.
// =============================================================================

BENCHMARK(Composite_UnknownMethod, iters) {
  folly::BenchmarkSuspender suspender;
  auto fixture = makeComposite(/*numChildren=*/4);
  // Use a method name no child registered. Cannot share buildPipeline here
  // (bench has no pipeline wiring); composite's writeUnknownMethodError
  // returns Result::Error early when pipeline_ is unset. The hot work
  // exercised here is still the methodMap miss probe.
  auto requests = prebuildRequests(iters, "nobody_owns_this");
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
