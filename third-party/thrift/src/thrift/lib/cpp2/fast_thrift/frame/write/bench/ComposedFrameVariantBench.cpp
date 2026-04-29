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

/*
 * ComposedFrameVariantBench compares the two discriminated-union options
 * holding the full set of 12 Composed*Frame alternatives, exercising the
 * three APIs the rocket pipeline actually calls (streamId, frameType,
 * serialize) plus per-instance size and move construction.
 *
 * Implementations compared:
 *   - std::variant            + std::visit
 *   - ComposedFrameVariant    + direct typed methods (no dispatch syntax
 *                               at the call site; fold-expression dispatch
 *                               is internal to the variant).
 *
 * For streamId / frameType the benchmark rotates through a pre-built
 * array of one variant per frame type. This prevents the compiler from
 * proving the call result is loop-invariant (which collapses the loop
 * body when the variant is held constant), and gives a realistic
 * dispatch-cost signal where the held alternative changes each iteration.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrameVariant.h>

#include <array>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>

using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::write;

namespace {

// =============================================================================
// Variant definitions — all 12 Composed*Frame alternatives in each.
// =============================================================================

#define ALL_COMPOSED_FRAMES                                             \
  ComposedRequestResponseFrame, ComposedRequestFnfFrame,                \
      ComposedRequestStreamFrame, ComposedRequestChannelFrame,          \
      ComposedRequestNFrame, ComposedCancelFrame, ComposedPayloadFrame, \
      ComposedErrorFrame, ComposedKeepAliveFrame, ComposedSetupFrame,   \
      ComposedMetadataPushFrame, ComposedExtFrame

using StdV = std::variant<ALL_COMPOSED_FRAMES>;
using CfvV = ComposedFrameVariant<ALL_COMPOSED_FRAMES>;

constexpr size_t kFrameCount = 12;

// =============================================================================
// Pre-built rotating arrays — one variant per frame type, indexed
// `i % kFrameCount` in the hot loop. Forces the compiler to actually
// dispatch each iteration; the held alternative changes per call so
// branch prediction sees a realistic distribution rather than always
// hitting the same arm.
// =============================================================================

std::array<StdV, kFrameCount> makeStdRotation() {
  return {{
      ComposedRequestResponseFrame{.header = {.streamId = 1}},
      ComposedRequestFnfFrame{.header = {.streamId = 2}},
      ComposedRequestStreamFrame{
          .header = {.streamId = 3, .initialRequestN = 1}},
      ComposedRequestChannelFrame{
          .header = {.streamId = 4, .initialRequestN = 1}},
      ComposedRequestNFrame{.header = {.streamId = 5, .requestN = 1}},
      ComposedCancelFrame{.header = {.streamId = 6}},
      ComposedPayloadFrame{.header = {.streamId = 7, .next = true}},
      ComposedErrorFrame{.header = {.streamId = 8, .errorCode = 0x201}},
      ComposedKeepAliveFrame{.header = {.lastReceivedPosition = 9}},
      ComposedSetupFrame{.header = {.majorVersion = 1, .keepaliveTime = 10}},
      ComposedMetadataPushFrame{},
      ComposedExtFrame{.header = {.streamId = 11, .extendedType = 1}},
  }};
}

std::array<CfvV, kFrameCount> makeCfvRotation() {
  return {{
      ComposedRequestResponseFrame{.header = {.streamId = 1}},
      ComposedRequestFnfFrame{.header = {.streamId = 2}},
      ComposedRequestStreamFrame{
          .header = {.streamId = 3, .initialRequestN = 1}},
      ComposedRequestChannelFrame{
          .header = {.streamId = 4, .initialRequestN = 1}},
      ComposedRequestNFrame{.header = {.streamId = 5, .requestN = 1}},
      ComposedCancelFrame{.header = {.streamId = 6}},
      ComposedPayloadFrame{.header = {.streamId = 7, .next = true}},
      ComposedErrorFrame{.header = {.streamId = 8, .errorCode = 0x201}},
      ComposedKeepAliveFrame{.header = {.lastReceivedPosition = 9}},
      ComposedSetupFrame{.header = {.majorVersion = 1, .keepaliveTime = 10}},
      ComposedMetadataPushFrame{},
      ComposedExtFrame{.header = {.streamId = 11, .extendedType = 1}},
  }};
}

// Helper: representative single-alternative variant (RequestResponse,
// hot-path) with empty IOBufs so per-iteration variant cost dominates
// over IOBuf allocation in the move/serialize benchmarks.
ComposedRequestResponseFrame makeRR() {
  return ComposedRequestResponseFrame{
      .data = nullptr,
      .metadata = nullptr,
      .header = {.streamId = 42, .follows = false},
  };
}

// =============================================================================
// streamId() — read-only, rotating through all 12 alternatives.
// =============================================================================

BENCHMARK(StdVariant_StreamId, n) {
  auto vs = makeStdRotation();
  for (unsigned i = 0; i < n; ++i) {
    auto& v = vs[i % kFrameCount];
    auto id = std::visit([](auto& p) { return p.streamId(); }, v);
    folly::doNotOptimizeAway(id);
  }
}

BENCHMARK_RELATIVE(ComposedFrameVariant_StreamId, n) {
  auto vs = makeCfvRotation();
  for (unsigned i = 0; i < n; ++i) {
    auto& v = vs[i % kFrameCount];
    folly::doNotOptimizeAway(v.streamId());
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// frameType() — read-only.
// ComposedFrameVariant returns the tag field directly (no dispatch).
// std::variant must dispatch through the type pack via std::visit.
// =============================================================================

BENCHMARK(StdVariant_FrameType, n) {
  auto vs = makeStdRotation();
  for (unsigned i = 0; i < n; ++i) {
    auto& v = vs[i % kFrameCount];
    auto t = std::visit(
        [](auto& p) { return std::remove_cvref_t<decltype(p)>::kFrameType; },
        v);
    folly::doNotOptimizeAway(t);
  }
}

BENCHMARK_RELATIVE(ComposedFrameVariant_FrameType, n) {
  auto vs = makeCfvRotation();
  for (unsigned i = 0; i < n; ++i) {
    auto& v = vs[i % kFrameCount];
    folly::doNotOptimizeAway(v.frameType());
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// serialize() — consuming. BenchmarkSuspender pauses the timer while we
// reconstruct the variant each iteration; only the dispatch + serialize
// is timed. Header-only frame (RequestResponse with null buffers) keeps
// the wire-bytes cost minimal so the variant dispatch is observable.
// =============================================================================

BENCHMARK(StdVariant_Serialize, n) {
  for (unsigned i = 0; i < n; ++i) {
    folly::BenchmarkSuspender s;
    StdV v = makeRR();
    s.dismiss();
    auto buf = std::visit(
        [](auto&& p) { return std::move(p).serialize(); }, std::move(v));
    folly::doNotOptimizeAway(buf);
  }
}

BENCHMARK_RELATIVE(ComposedFrameVariant_Serialize, n) {
  for (unsigned i = 0; i < n; ++i) {
    folly::BenchmarkSuspender s;
    CfvV v = makeRR();
    s.dismiss();
    auto buf = std::move(v).serialize();
    folly::doNotOptimizeAway(buf);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Move construction — measures variant-internal move-dispatch cost.
// =============================================================================

BENCHMARK(StdVariant_MoveConstruct, n) {
  for (unsigned i = 0; i < n; ++i) {
    StdV src = makeRR();
    StdV dst{std::move(src)};
    folly::doNotOptimizeAway(dst);
  }
}

BENCHMARK_RELATIVE(ComposedFrameVariant_MoveConstruct, n) {
  for (unsigned i = 0; i < n; ++i) {
    CfvV src = makeRR();
    CfvV dst{std::move(src)};
    folly::doNotOptimizeAway(dst);
  }
}

} // namespace

// =============================================================================

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  LOG(INFO) << "sizeof(std::variant<12 frames>):              " << sizeof(StdV);
  LOG(INFO) << "sizeof(ComposedFrameVariant<12 frames>):      " << sizeof(CfvV);

  folly::runBenchmarks();
  return 0;
}
