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

#include <glog/logging.h>

#include <gtest/gtest.h>
#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/async/metadata/RequestRpcMetadataFacade.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::test {
std::unique_ptr<folly::IOBuf> initData() {
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;
  metadata.kind() = RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.name() = "foo";
  metadata.clientTimeoutMs() = 10000;
  metadata.queueTimeoutMs() = 10000;
  metadata.priority() = RpcPriority::NORMAL;
  metadata.clientId() = "client";
  metadata.otherMetadata() = {{"key", "value"}};
  CompactProtocolWriter writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

BENCHMARK(CompactProtocolRead, iters) {
  folly::BenchmarkSuspender braces;
  while (iters--) {
    auto buf = initData();
    braces.dismiss();
    CompactProtocolReader reader;
    reader.setInput(buf.get());
    RequestRpcMetadata metadata;
    metadata.read(&reader);
    folly::doNotOptimizeAway(metadata.protocol().value());
    folly::doNotOptimizeAway(metadata.kind().value());
    folly::doNotOptimizeAway(metadata.name().value());
    folly::doNotOptimizeAway(metadata.clientTimeoutMs().value());
    folly::doNotOptimizeAway(metadata.queueTimeoutMs().value());
    folly::doNotOptimizeAway(metadata.priority().value());
    folly::doNotOptimizeAway(metadata.clientId().value());
    folly::doNotOptimizeAway(metadata.otherMetadata().value());
    braces.rehire();
  }
}

BENCHMARK(CompactProtocolReadWithFacadeMove, iters) {
  folly::BenchmarkSuspender braces;
  while (iters--) {
    auto buf = initData();
    braces.dismiss();
    RequestRpcMetadataFacade facade(std::move(buf));
    folly::doNotOptimizeAway(facade.protocolId().value());
    folly::doNotOptimizeAway(facade.kind().value());
    folly::doNotOptimizeAway(facade.name().value());
    folly::doNotOptimizeAway(facade.clientTimeoutMs().value());
    folly::doNotOptimizeAway(facade.queueTimeoutMs().value());
    folly::doNotOptimizeAway(facade.priority().value());
    folly::doNotOptimizeAway(facade.clientId().value());
    folly::doNotOptimizeAway(facade.otherMetadata().value());
    braces.rehire();
  }
}

BENCHMARK(CompactProtocolReadWithFacadeRef, iters) {
  folly::BenchmarkSuspender braces;
  while (iters--) {
    auto buf = initData();
    braces.dismiss();
    RequestRpcMetadataFacade facade(*buf);
    folly::doNotOptimizeAway(facade.protocolId().value());
    folly::doNotOptimizeAway(facade.kind().value());
    folly::doNotOptimizeAway(facade.name().value());
    folly::doNotOptimizeAway(facade.clientTimeoutMs().value());
    folly::doNotOptimizeAway(facade.queueTimeoutMs().value());
    folly::doNotOptimizeAway(facade.priority().value());
    folly::doNotOptimizeAway(facade.clientId().value());
    folly::doNotOptimizeAway(facade.otherMetadata().value());
    braces.rehire();
  }
}

} // namespace apache::thrift::test
int main(int argc, char* argv[]) {
  folly::Init init(&argc, &argv);
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  apache::thrift::test::initData();
  folly::runBenchmarks();
  return 0;
}
