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
 * AcceptancePipelineIntegrationBench
 *
 * Measures the per-connection cost of a full acceptance pipeline trip:
 *   ConnectionListener → MockTailHandler
 * (fd wrap, fireRead, take<>, tail dispatch). Drives connectionAccepted()
 * directly so we don't pay bind/listen + kernel-accept costs.
 */

#include <sys/socket.h>
#include <array>
#include <vector>

#include <folly/Benchmark.h>
#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/net/NetworkSocket.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/connection/endpoint/ConnectionListener.h>

using namespace folly;
using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator;
using apache::thrift::fast_thrift::channel_pipeline::test::MockTailHandler;
using apache::thrift::fast_thrift::connection::ConnectionListener;
using apache::thrift::fast_thrift::connection::SocketOptions;

namespace {

std::pair<folly::NetworkSocket, folly::NetworkSocket> makeSocketPair() {
  std::array<int, 2> fds{};
  PCHECK(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds.data()) == 0);
  return {folly::NetworkSocket(fds[0]), folly::NetworkSocket(fds[1])};
}

} // namespace

BENCHMARK(Plaintext_AcceptToTail, iters) {
  BenchmarkSuspender suspender;

  ScopedEventBaseThread evbThread;
  auto* evb = evbThread.getEventBase();

  MockTailHandler tail;
  SimpleBufferAllocator allocator;
  ConnectionListener::Ptr listener(new ConnectionListener(
      evb,
      folly::SocketAddress("::1", 0),
      SocketOptions{},
      /*enableReusePortBpfSpread=*/false));
  auto pipeline = PipelineBuilder<
                      ConnectionListener,
                      MockTailHandler,
                      SimpleBufferAllocator>()
                      .setEventBase(evb)
                      .setHead(listener.get())
                      .setTail(&tail)
                      .setAllocator(&allocator)
                      .build();
  listener->setPipeline(pipeline.get());
  evb->runInEventBaseThreadAndWait([&] { pipeline->activate(); });

  std::vector<folly::NetworkSocket> serverFds(iters);
  std::vector<folly::NetworkSocket> clientFds(iters);
  for (size_t i = 0; i < iters; ++i) {
    auto p = makeSocketPair();
    clientFds[i] = p.first;
    serverFds[i] = p.second;
  }
  folly::SocketAddress clientAddr("127.0.0.1", 5001);

  suspender.dismiss();
  evb->runInEventBaseThreadAndWait([&] {
    for (size_t i = 0; i < iters; ++i) {
      listener->connectionAccepted(
          serverFds[i],
          clientAddr,
          folly::AsyncServerSocket::AcceptCallback::AcceptInfo{});
    }
  });
  suspender.rehire();

  doNotOptimizeAway(tail.readCount());
  for (auto fd : clientFds) {
    ::close(fd.toFd());
  }
  evb->runInEventBaseThreadAndWait([&] {
    listener->resetPipeline();
    pipeline.reset();
    listener.reset();
  });
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
