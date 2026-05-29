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
 * RocketClientErrorFrameHandler Microbenchmarks
 *
 * This handler does:
 * - onRead: Checks if frame is connection-level ERROR (streamId == 0)
 *   - If so: extracts error code, creates exception, fires exception
 *   - Otherwise: pass-through
 *
 * Meaningful benchmarks:
 * - Connection-level ERROR handling: Exception creation overhead
 * - Pass-through: Non-error frames and stream-level errors
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/logging/Init.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::rocket;
using namespace apache::thrift::fast_thrift::rocket::client::handler;

namespace {

using rocket::bench::BenchContext;

// =============================================================================
// Helper Functions
// =============================================================================

RocketResponseMessage createConnectionErrorFrame(ErrorCode errorCode) {
  auto frame = serialize(
      ErrorHeader{
          .streamId = 0, // Connection-level
          .errorCode = static_cast<uint32_t>(errorCode)},
      nullptr,
      folly::IOBuf::copyBuffer("error"));

  return RocketResponseMessage{
      .frame = parseFrame(std::move(frame)),
      .requestFrameType = FrameType::RESERVED,
  };
}

// =============================================================================
// Connection-Level ERROR Frame Benchmarks - Exception creation overhead
// =============================================================================

BENCHMARK(Read_ErrorHandler_ConnectionClose, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientErrorFrameHandler handler;
  BenchContext ctx;

  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        createConnectionErrorFrame(ErrorCode::CONNECTION_CLOSE));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_ErrorHandler_InvalidSetup, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientErrorFrameHandler handler;
  BenchContext ctx;

  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(createConnectionErrorFrame(ErrorCode::INVALID_SETUP));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_ErrorHandler_RejectedSetup, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientErrorFrameHandler handler;
  BenchContext ctx;

  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(createConnectionErrorFrame(ErrorCode::REJECTED_SETUP));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

} // namespace

int main(int argc, char** argv) {
  // Suppress error logging during benchmark
  folly::initLogging(".=FATAL");
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
