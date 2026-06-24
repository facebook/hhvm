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
 * ThriftClientChecksumHandler Microbenchmarks
 *
 * Two things matter for this handler:
 *
 *   1. Pass-through cost when no checksum is requested (the common case in
 *      production). Must be near-free — the handler is unconditionally
 *      installed in the outbound thrift pipeline.
 *
 *   2. Compute cost for XXH3_64 across representative payload sizes. Drives
 *      the "is checksumming worth enabling for this RPC" decision. XXH3_64
 *      is the only supported algorithm; CRC32 / SERVER_ONLY_CRC32 are not.
 */

#include <cstring>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/PayloadVariants.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientChecksumHandler.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::thrift;
using namespace apache::thrift::fast_thrift::thrift::client::handler;

namespace {

class BenchContext {
 public:
  Result fireWrite(TypeErasedBox&& msg) noexcept {
    last_ = std::move(msg);
    return Result::Success;
  }

 private:
  TypeErasedBox last_;
};

std::unique_ptr<folly::IOBuf> makeData(size_t bytes) {
  auto buf = folly::IOBuf::create(bytes);
  std::memset(buf->writableTail(), 'x', bytes);
  buf->append(bytes);
  return buf;
}

std::unique_ptr<apache::thrift::RequestRpcMetadata> emptyMetadata() {
  return std::make_unique<apache::thrift::RequestRpcMetadata>();
}

std::unique_ptr<apache::thrift::RequestRpcMetadata> metadataWithAlgorithm(
    apache::thrift::ChecksumAlgorithm algo) {
  auto md = std::make_unique<apache::thrift::RequestRpcMetadata>();
  apache::thrift::Checksum c;
  c.algorithm() = algo;
  md->checksum() = c;
  return md;
}

ThriftRequestMessage makeRequest(
    std::unique_ptr<apache::thrift::RequestRpcMetadata> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  return ThriftRequestMessage{
      .payload =
          ThriftRequestResponsePayload{
              .data = std::move(data),
              .metadata = std::move(metadata),
          },
      .requestContext = {},
  };
}

template <typename MetadataFactory>
void runBench(
    size_t iters, size_t payloadBytes, MetadataFactory&& makeMetadata) {
  folly::BenchmarkSuspender suspender;
  ThriftClientChecksumHandler handler;
  BenchContext ctx;

  auto template_data = makeData(payloadBytes);
  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.push_back(
        erase_and_box(makeRequest(makeMetadata(), template_data->clone())));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, std::move(messages[i]));
    folly::doNotOptimizeAway(result);
  }
}

} // namespace

// =============================================================================
// Pass-through (no checksum requested) — must be near-free
// =============================================================================

BENCHMARK(PassThrough_NoChecksum_256B, iters) {
  runBench(iters, 256, [] { return emptyMetadata(); });
}

BENCHMARK(PassThrough_NoChecksum_4KB, iters) {
  runBench(iters, 4096, [] { return emptyMetadata(); });
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Checksum struct path — XXH3_64 across representative payload sizes
// =============================================================================

BENCHMARK(ChecksumStruct_XXH3_64_256B, iters) {
  runBench(iters, 256, [] {
    return metadataWithAlgorithm(apache::thrift::ChecksumAlgorithm::XXH3_64);
  });
}

BENCHMARK(ChecksumStruct_XXH3_64_4KB, iters) {
  runBench(iters, 4096, [] {
    return metadataWithAlgorithm(apache::thrift::ChecksumAlgorithm::XXH3_64);
  });
}

BENCHMARK(ChecksumStruct_XXH3_64_64KB, iters) {
  runBench(iters, 64 * 1024, [] {
    return metadataWithAlgorithm(apache::thrift::ChecksumAlgorithm::XXH3_64);
  });
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
