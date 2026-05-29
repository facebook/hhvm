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
 * FastThriftServer End-to-End Benchmark
 *
 * Measures request-response throughput of FastThriftServer vs ThriftServer
 * using simple RPCs. Both servers use the same handler and the same
 * RocketClientChannel-based client, so the comparison isolates server-side
 * differences.
 */

#include <gflags/gflags.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestServiceAsyncClient.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::bench {
namespace {

using apache::thrift::fast_thrift::thrift::test::
    BackwardsCompatibilityTestService;

using ClientType = apache::thrift::Client<BackwardsCompatibilityTestService>;

class BenchHandler
    : public apache::thrift::ServiceHandler<BackwardsCompatibilityTestService> {
 public:
  void ping() override {}

  void echo(
      std::string& response, std::unique_ptr<std::string> message) override {
    response = *message;
  }

  int64_t add(int64_t a, int64_t b) override { return a + b; }

  void sendResponse(std::string& response, int64_t size) override {
    response = std::string(static_cast<size_t>(size), 'x');
  }
};

void callPing(folly::EventBase* evb, ClientType& client) {
  folly::Baton<> baton;
  evb->runInEventBaseThread([&] {
    client.semifuture_ping()
        .via(evb)
        .thenValue([&](folly::Unit) { baton.post(); })
        .thenError([&](const folly::exception_wrapper&) { baton.post(); });
  });
  baton.wait();
}

std::string callEcho(
    folly::EventBase* evb, ClientType& client, const std::string& msg) {
  folly::Baton<> baton;
  std::string result;
  evb->runInEventBaseThread([&] {
    client.semifuture_echo(msg)
        .via(evb)
        .thenValue([&](std::string val) {
          result = std::move(val);
          baton.post();
        })
        .thenError([&](const folly::exception_wrapper&) { baton.post(); });
  });
  baton.wait();
  return result;
}

int64_t callAdd(
    folly::EventBase* evb, ClientType& client, int64_t a, int64_t b) {
  folly::Baton<> baton;
  int64_t result = 0;
  evb->runInEventBaseThread([&] {
    client.semifuture_add(a, b)
        .via(evb)
        .thenValue([&](int64_t val) {
          result = val;
          baton.post();
        })
        .thenError([&](const folly::exception_wrapper&) { baton.post(); });
  });
  baton.wait();
  return result;
}

std::unique_ptr<ClientType> createClient(
    folly::EventBase* evb, const folly::SocketAddress& address) {
  std::unique_ptr<ClientType> client;
  evb->runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(evb, address);
    auto channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(socket));
    client = std::make_unique<ClientType>(std::move(channel));
  });
  return client;
}

void destroyClient(folly::EventBase* evb, std::unique_ptr<ClientType>& client) {
  evb->runInEventBaseThreadAndWait([&] { client.reset(); });
}

// -- FastThriftServer fixture --

struct FastThriftServerFixture {
  FastThriftServerFixture(const FastThriftServerFixture&) = delete;
  FastThriftServerFixture& operator=(const FastThriftServerFixture&) = delete;
  FastThriftServerFixture(FastThriftServerFixture&&) = delete;
  FastThriftServerFixture& operator=(FastThriftServerFixture&&) = delete;

  FastThriftServerFixture() {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler = std::make_shared<BenchHandler>();

    thrift::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;

    server =
        std::make_unique<thrift::FastThriftServer>(std::move(config), handler);
    server->start();

    clientThread = std::make_unique<folly::ScopedEventBaseThread>();
    client = createClient(clientThread->getEventBase(), server->getAddress());
  }

  ~FastThriftServerFixture() {
    destroyClient(clientThread->getEventBase(), client);
    clientThread.reset();
    server->stop();
    server.reset();
  }

  std::shared_ptr<BenchHandler> handler;
  std::unique_ptr<thrift::FastThriftServer> server;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread;
  std::unique_ptr<ClientType> client;
};

// -- ThriftServer fixture --

struct ThriftServerFixture {
  ThriftServerFixture(const ThriftServerFixture&) = delete;
  ThriftServerFixture& operator=(const ThriftServerFixture&) = delete;
  ThriftServerFixture(ThriftServerFixture&&) = delete;
  ThriftServerFixture& operator=(ThriftServerFixture&&) = delete;

  ThriftServerFixture() {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler = std::make_shared<BenchHandler>();

    server = std::make_shared<apache::thrift::ThriftServer>();
    server->setInterface(handler);
    server->setAddress(folly::SocketAddress("::1", 0));
    server->setNumIOWorkerThreads(1);
    server->setNumCPUWorkerThreads(1);

    serverThread = std::make_unique<std::thread>([this] { server->serve(); });

    // Wait for server to be ready
    while (server->getAddress().getPort() == 0) {
      /* sleep override */
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    clientThread = std::make_unique<folly::ScopedEventBaseThread>();
    client = createClient(clientThread->getEventBase(), server->getAddress());
  }

  ~ThriftServerFixture() {
    destroyClient(clientThread->getEventBase(), client);
    clientThread.reset();
    server->stop();
    serverThread->join();
  }

  std::shared_ptr<BenchHandler> handler;
  std::shared_ptr<apache::thrift::ThriftServer> server;
  std::unique_ptr<std::thread> serverThread;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread;
  std::unique_ptr<ClientType> client;
};

// -- Benchmarks --

BENCHMARK(FastThriftServer_Ping, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftServerFixture fixture;
  auto* evb = fixture.clientThread->getEventBase();
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    callPing(evb, *fixture.client);
  }
}

BENCHMARK(ThriftServer_Ping, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftServerFixture fixture;
  auto* evb = fixture.clientThread->getEventBase();
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    callPing(evb, *fixture.client);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(FastThriftServer_Echo, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftServerFixture fixture;
  auto* evb = fixture.clientThread->getEventBase();
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    callEcho(evb, *fixture.client, "hello");
  }
}

BENCHMARK(ThriftServer_Echo, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftServerFixture fixture;
  auto* evb = fixture.clientThread->getEventBase();
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    callEcho(evb, *fixture.client, "hello");
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(FastThriftServer_Add, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftServerFixture fixture;
  auto* evb = fixture.clientThread->getEventBase();
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    callAdd(evb, *fixture.client, 17, 25);
  }
}

BENCHMARK(ThriftServer_Add, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftServerFixture fixture;
  auto* evb = fixture.clientThread->getEventBase();
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    callAdd(evb, *fixture.client, 17, 25);
  }
}

} // namespace
} // namespace apache::thrift::fast_thrift::bench

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
