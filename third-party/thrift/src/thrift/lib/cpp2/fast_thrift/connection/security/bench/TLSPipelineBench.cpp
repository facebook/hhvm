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

// End-to-end benchmark for the inner TLS pipeline. Measures full
// per-connection cost (peek + fizz handshake + emit) through the whole
// chain owned by ConnectionTLSHandler. Reuses the test cert fixture; each
// iteration spawns a fresh socket pair, drives one handshake to
// completion, and waits for the ConnectionMessage to surface at the tail.

#include <sys/socket.h>
#include <array>
#include <chrono>
#include <memory>

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/FizzClientContext.h>
#include <folly/Benchmark.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/net/NetworkSocket.h>
#include <folly/observer/SimpleObservable.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/handler/ConnectionTLSHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/security/SSLPolicy.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/test/TestCert.h>

namespace apache::thrift::fast_thrift::connection::security::bench {

namespace fts = ::apache::thrift::fast_thrift::security;
namespace fts_test = ::apache::thrift::fast_thrift::security::test;
namespace conn = ::apache::thrift::fast_thrift::connection;

namespace {

class NoopHead {
 public:
  channel_pipeline::Result onWrite(channel_pipeline::TypeErasedBox&&) noexcept {
    return channel_pipeline::Result::Success;
  }
  void onReadReady() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
};

class CapturingTail {
 public:
  using OnRead = folly::Function<void(conn::ConnectionMessage&&) noexcept>;
  explicit CapturingTail(OnRead onRead) noexcept : onRead_(std::move(onRead)) {}

  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto m = msg.take<conn::ConnectionMessage>();
    if (onRead_) {
      onRead_(std::move(m));
    }
    return channel_pipeline::Result::Success;
  }
  void onException(folly::exception_wrapper&&) noexcept {}
  void onWriteReady() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}

 private:
  OnRead onRead_;
};

class TestFizzClient
    : private fizz::client::AsyncFizzClient::HandshakeCallback {
 public:
  using DoneCallback = folly::Function<void(folly::exception_wrapper) noexcept>;

  TestFizzClient(folly::EventBase* evb, folly::NetworkSocket fd) {
    auto sock = folly::AsyncSocket::newSocket(evb, fd);
    auto ctx = std::make_shared<fizz::client::FizzClientContext>();
    client_.reset(new fizz::client::AsyncFizzClient(std::move(sock), ctx));
  }

  void start(DoneCallback done) {
    done_ = std::move(done);
    client_->connect(this, nullptr, folly::none, folly::none, folly::none);
  }

 private:
  void fizzHandshakeSuccess(fizz::client::AsyncFizzClient*) noexcept override {
    if (done_) {
      done_(folly::exception_wrapper());
    }
  }
  void fizzHandshakeError(
      fizz::client::AsyncFizzClient*,
      folly::exception_wrapper ex) noexcept override {
    if (done_) {
      done_(std::move(ex));
    }
  }

  fizz::client::AsyncFizzClient::UniquePtr client_;
  DoneCallback done_;
};

HANDLER_TAG(tls_bench_handler);

// Holds the server-side pipeline once; reused across iterations. Built once
// in main() to keep the benchmark loop measuring only the per-handshake cost.
struct Harness {
  explicit Harness(fts::SSLPolicy policy) {
    evbThread = std::make_unique<folly::ScopedEventBaseThread>();
    evb = evbThread->getEventBase();

    fts::FizzServerCertConfig cfg;
    auto cert = fts_test::makeTestCert();
    cfg.certPem = cert.certPem;
    cfg.keyPem = cert.keyPem;
    cfg.clientAuth = fizz::server::ClientAuthMode::None;
    tlsParams = std::make_shared<const fts::TLSParams>(
        fts::buildTLSParams(cfg, fts::ThriftTlsConfig{}));
    tlsParamsObservable = std::make_unique<folly::observer::SimpleObservable<
        std::shared_ptr<const fts::TLSParams>>>(tlsParams);

    head = std::make_unique<NoopHead>();
    tail = std::make_unique<CapturingTail>(
        [this](conn::ConnectionMessage&& m) noexcept {
          // Drop the transport on the EVB; signal completion.
          lastTransport = std::move(m.transport);
          done.post();
        });

    evb->runInEventBaseThreadAndWait([&] {
      channel_pipeline::PipelineBuilder<
          NoopHead,
          CapturingTail,
          channel_pipeline::SimpleBufferAllocator>
          builder;
      builder.setEventBase(evb)
          .setHead(head.get())
          .setTail(tail.get())
          .setAllocator(&allocator)
          .addNextDuplex<conn::handler::ConnectionTLSHandler>(
              tls_bench_handler_tag,
              *evb,
              policy,
              tlsParamsObservable->getObserver(),
              &allocator);
      pipeline = builder.build();
      pipeline->activate();
    });
  }

  ~Harness() {
    if (pipeline) {
      evb->runInEventBaseThreadAndWait([&] {
        pipeline->deactivate();
        pipeline.reset();
      });
    }
    evbThread.reset();
  }

  Harness(const Harness&) = delete;
  Harness& operator=(const Harness&) = delete;
  Harness(Harness&&) = delete;
  Harness& operator=(Harness&&) = delete;

  // One iteration: socket pair → feed server → drive client → wait for emit.
  void runOne() {
    std::array<int, 2> fds{};
    PCHECK(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds.data()) == 0);
    folly::NetworkSocket serverFd(fds[0]);
    folly::NetworkSocket clientFd(fds[1]);

    done.reset();
    std::unique_ptr<TestFizzClient> client;

    evb->runInEventBaseThreadAndWait([&] {
      auto serverSock = folly::AsyncSocket::newSocket(evb, serverFd);
      conn::ConnectionMessage msg{
          .transport = folly::AsyncTransport::UniquePtr(serverSock.release()),
          .clientAddr = folly::SocketAddress{"127.0.0.1", 0},
      };
      (void)pipeline->fireRead(channel_pipeline::erase_and_box(std::move(msg)));

      client = std::make_unique<TestFizzClient>(evb, clientFd);
      client->start([](const folly::exception_wrapper&) noexcept {});
    });

    done.wait();

    evb->runInEventBaseThreadAndWait([&] {
      lastTransport.reset();
      client.reset();
    });
  }

  std::unique_ptr<folly::ScopedEventBaseThread> evbThread;
  folly::EventBase* evb{nullptr};
  std::shared_ptr<const fts::TLSParams> tlsParams;
  std::unique_ptr<
      folly::observer::SimpleObservable<std::shared_ptr<const fts::TLSParams>>>
      tlsParamsObservable;
  channel_pipeline::SimpleBufferAllocator allocator;
  std::unique_ptr<NoopHead> head;
  std::unique_ptr<CapturingTail> tail;
  channel_pipeline::PipelineImpl::Ptr pipeline;
  folly::Baton<> done;
  folly::AsyncTransport::UniquePtr lastTransport;
};

// Lazily-constructed singletons so the benchmark loop measures only the
// per-handshake cost. Built on first access; destroyed at program exit.
Harness& requiredHarness() {
  static Harness h(fts::SSLPolicy::REQUIRED);
  return h;
}
Harness& permittedHarness() {
  static Harness h(fts::SSLPolicy::PERMITTED);
  return h;
}

} // namespace

BENCHMARK(RequiredHandshake, n) {
  auto& h = requiredHarness();
  for (size_t i = 0; i < n; ++i) {
    h.runOne();
  }
}

BENCHMARK_RELATIVE(PermittedHandshake, n) {
  auto& h = permittedHarness();
  for (size_t i = 0; i < n; ++i) {
    h.runOne();
  }
}

} // namespace apache::thrift::fast_thrift::connection::security::bench

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
