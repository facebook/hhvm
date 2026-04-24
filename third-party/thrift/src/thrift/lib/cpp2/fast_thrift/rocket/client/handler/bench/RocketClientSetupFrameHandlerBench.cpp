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
 * RocketClientSetupFrameHandler Microbenchmarks
 *
 * This handler does:
 * - onConnect: Calls factory function, serializes SETUP frame via serialize()
 * - onRead: Pure pass-through (no work done)
 *
 * Meaningful benchmarks:
 * - Connect path: Setup frame serialization overhead
 * - Read path: Pass-through overhead (should be minimal)
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>

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

RocketClientSetupFrameHandler::SetupFactory minimalSetupFactory() {
  return []() {
    return std::make_pair(
        folly::IOBuf::copyBuffer("metadata"), std::unique_ptr<folly::IOBuf>());
  };
}

RocketClientSetupFrameHandler::SetupFactory setupWithDataFactory() {
  return []() {
    return std::make_pair(
        folly::IOBuf::copyBuffer("metadata"), folly::IOBuf::copyBuffer("data"));
  };
}

RocketClientSetupFrameHandler::SetupFactory noMetadataSetupFactory() {
  return []() {
    return std::make_pair(
        std::unique_ptr<folly::IOBuf>(), std::unique_ptr<folly::IOBuf>());
  };
}

// =============================================================================
// Connect Path Benchmarks - SETUP frame serialization
// =============================================================================

BENCHMARK(Connect_SetupHandler_MinimalMetadata, iters) {
  BenchContext ctx;

  for (size_t i = 0; i < iters; ++i) {
    RocketClientSetupFrameHandler handler(minimalSetupFactory());
    handler.onPipelineActive(ctx);
  }
}

BENCHMARK(Connect_SetupHandler_WithData, iters) {
  BenchContext ctx;

  for (size_t i = 0; i < iters; ++i) {
    RocketClientSetupFrameHandler handler(setupWithDataFactory());
    handler.onPipelineActive(ctx);
  }
}

BENCHMARK(Connect_SetupHandler_NoMetadata, iters) {
  BenchContext ctx;

  for (size_t i = 0; i < iters; ++i) {
    RocketClientSetupFrameHandler handler(noMetadataSetupFactory());
    handler.onPipelineActive(ctx);
  }
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
