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
 * Request Metadata Deserialization Benchmark
 *
 * Measures the performance gain of eliminating the
 * make_unique<RequestRpcMetadata> (~150B) heap allocation per inbound request.
 *
 * Compares two paths:
 * - HeapAlloc (old): deserialize metadata from frame -> make_unique -> extract
 *   fields -> discard. This was the old handler + channel path.
 * - StackOnly (new): deserialize metadata on stack from frame -> extract fields
 *   directly. This is the merged ThriftServerChannel path.
 *
 * Both paths deserialize from the same pre-built frames, so the comparison
 * isolates the cost of the heap allocation and indirection.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;

namespace {

// =============================================================================
// Helpers
// =============================================================================

SimpleBufferAllocator g_allocator;

BytesPtr copyBuffer(folly::StringPiece s) {
  auto buf = g_allocator.allocate(s.size());
  std::memcpy(buf->writableData(), s.data(), s.size());
  buf->append(s.size());
  return buf;
}

ParsedFrame createRequestFrame(
    const apache::thrift::RequestRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);

  auto buf = serialize(
      RequestResponseHeader{.streamId = 1},
      queue.move(),
      copyBuffer("payload"));
  return parseFrame(std::move(buf));
}

// =============================================================================
// Metadata templates
// =============================================================================

apache::thrift::RequestRpcMetadata createMinimalMetadata() {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = "testMethod";
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
  return metadata;
}

apache::thrift::RequestRpcMetadata createTypicalMetadata() {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = "testMethod";
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
  metadata.clientTimeoutMs() = 10000;
  metadata.queueTimeoutMs() = 5000;

  auto& headers = metadata.otherMetadata().ensure();
  headers["request_id"] = "abc123";
  headers["trace_id"] = "trace-456";
  headers["client_name"] = "benchmark_client";

  return metadata;
}

apache::thrift::RequestRpcMetadata createLargeMetadata() {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = "testMethodWithLongName";
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
  metadata.clientTimeoutMs() = 10000;
  metadata.queueTimeoutMs() = 5000;

  auto& headers = metadata.otherMetadata().ensure();
  for (int i = 0; i < 20; ++i) {
    headers[fmt::format("header_{}", i)] = fmt::format("value_{}", i);
  }

  return metadata;
}

// =============================================================================
// Benchmark implementations
// =============================================================================

/**
 * Old path: deserialize metadata from frame, heap-allocate via make_unique,
 * extract fields through the pointer indirection, then discard.
 */
void benchHeapAlloc(
    size_t iters, const apache::thrift::RequestRpcMetadata& metadataTemplate) {
  BenchmarkSuspender suspender;

  std::vector<ParsedFrame> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createRequestFrame(metadataTemplate));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto& frame = frames[i];

    // Deserialize
    apache::thrift::RequestRpcMetadata metadata;
    apache::thrift::BinaryProtocolReader reader;
    reader.setInput(frame.metadataCursor());
    metadata.read(&reader);

    // Heap allocate (simulates old make_unique into CompactVariant)
    auto heapMeta = std::make_unique<apache::thrift::RequestRpcMetadata>(
        std::move(metadata));

    // Extract fields through pointer (simulates channel consumption)
    doNotOptimizeAway(heapMeta->name());
    doNotOptimizeAway(heapMeta->kind());
    doNotOptimizeAway(heapMeta->protocol());
  }
}

/**
 * New path: deserialize metadata directly on the stack from the frame,
 * extract fields with no indirection. Zero heap allocation.
 */
void benchStackOnly(
    size_t iters, const apache::thrift::RequestRpcMetadata& metadataTemplate) {
  BenchmarkSuspender suspender;

  std::vector<ParsedFrame> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createRequestFrame(metadataTemplate));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto& frame = frames[i];

    // Deserialize directly on stack (no heap alloc)
    apache::thrift::RequestRpcMetadata metadata;
    apache::thrift::BinaryProtocolReader reader;
    reader.setInput(frame.metadataCursor());
    metadata.read(&reader);

    // Extract fields directly (no pointer indirection)
    doNotOptimizeAway(metadata.name());
    doNotOptimizeAway(metadata.kind());
    doNotOptimizeAway(metadata.protocol());
  }
}

// =============================================================================
// Minimal metadata
// =============================================================================

BENCHMARK(HeapAlloc_Minimal, iters) {
  benchHeapAlloc(iters, createMinimalMetadata());
}

BENCHMARK_RELATIVE(StackOnly_Minimal, iters) {
  benchStackOnly(iters, createMinimalMetadata());
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Typical metadata (3 headers + timeouts)
// =============================================================================

BENCHMARK(HeapAlloc_Typical, iters) {
  benchHeapAlloc(iters, createTypicalMetadata());
}

BENCHMARK_RELATIVE(StackOnly_Typical, iters) {
  benchStackOnly(iters, createTypicalMetadata());
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Large metadata (20 headers)
// =============================================================================

BENCHMARK(HeapAlloc_ManyHeaders, iters) {
  benchHeapAlloc(iters, createLargeMetadata());
}

BENCHMARK_RELATIVE(StackOnly_ManyHeaders, iters) {
  benchStackOnly(iters, createLargeMetadata());
}

} // namespace

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
