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
 * FizzHandshakeHandler Microbenchmark
 *
 * The handshake itself is dominated by Fizz crypto and async I/O; we cannot
 * meaningfully benchmark that here. What we CAN measure is the synchronous
 * onRead path: take<ConnectionMessage>(), AsyncSocket rewrap, build the
 * FizzHandshakeHelper, register it in PendingHandshakes, and call start().
 *
 * The benchmark cancels all in-flight handshakes after measurement so we
 * don't leak helpers between iterations.
 */

#include <sys/socket.h>
#include <array>
#include <chrono>

#include <folly/Benchmark.h>
#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/net/NetworkSocket.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/endpoint/ConnectionListener.h>
#include <thrift/lib/cpp2/fast_thrift/connection/handler/FizzHandshakeHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/test/TestCert.h>

using namespace folly;
using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator;
using apache::thrift::fast_thrift::channel_pipeline::test::MockTailHandler;
using apache::thrift::fast_thrift::connection::ConnectionListener;
using apache::thrift::fast_thrift::connection::SocketOptions;
using apache::thrift::fast_thrift::connection::handler::FizzHandshakeHandler;

HANDLER_TAG(fizz_handler);

namespace {

std::pair<folly::NetworkSocket, folly::NetworkSocket> makeSocketPair() {
  std::array<int, 2> fds{};
  PCHECK(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds.data()) == 0);
  return {folly::NetworkSocket(fds[0]), folly::NetworkSocket(fds[1])};
}

std::shared_ptr<const fizz::server::FizzServerContext> makeServerContext() {
  apache::thrift::fast_thrift::security::FizzServerCertConfig cfg;
  auto cert = apache::thrift::fast_thrift::security::test::makeTestCert();
  cfg.certPem = cert.certPem;
  cfg.keyPem = cert.keyPem;
  cfg.clientAuth = fizz::server::ClientAuthMode::None;
  return apache::thrift::fast_thrift::security::buildFizzServerContext(
             cfg, apache::thrift::fast_thrift::security::ThriftTlsConfig{})
      .fizzContext;
}

} // namespace

// Measures: connectionAccepted → fireRead → FizzHandshakeHandler.onRead →
// FizzHandshakeHelper construction + register + start(). Does NOT measure
// crypto / handshake completion — those are async and dominated by external
// work.
BENCHMARK(OnRead_StartFizzHandshake, iters) {
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
                      .addNextDuplex<FizzHandshakeHandler>(
                          fizz_handler_tag,
                          makeServerContext(),
                          /*thriftParams=*/nullptr,
                          std::chrono::seconds{30})
                      .build();
  listener->setPipeline(pipeline.get());
  evb->runInEventBaseThreadAndWait([&] { pipeline->activate(); });

  std::vector<folly::NetworkSocket> serverFds;
  std::vector<folly::NetworkSocket> clientFds;
  serverFds.reserve(iters);
  clientFds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    auto [c, s] = makeSocketPair();
    clientFds.push_back(c);
    serverFds.push_back(s);
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

  // Tear down: cancel all pending handshakes via onPipelineInactive (fires
  // on pipeline destruction), then close the client side.
  evb->runInEventBaseThreadAndWait([&] {
    listener->resetPipeline();
    pipeline.reset();
    listener.reset();
  });
  for (auto fd : clientFds) {
    ::close(fd.toFd());
  }
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
