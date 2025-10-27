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

#include <folly/Benchmark.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/DetachOnCancel.h>
#include <folly/init/Init.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using namespace apache::thrift::test;

class DummyCallback : public RequestCallback {
 public:
  explicit DummyCallback(ClientReceiveState& state, folly::Baton<>& baton)
      : state_(state), baton_(baton) {}
  void requestSent() override {}
  void replyReceived(ClientReceiveState&& state) override {
    state_ = std::move(state);
    baton_.post();
  }
  void requestError(ClientReceiveState&&) override {}
  bool isInlineSafe() const override { return true; }
  ClientReceiveState& state_;

 private:
  folly::Baton<>& baton_;
};

class DummyChannel : public apache::thrift::RequestChannel {
  void sendRequestResponse(
      const RpcOptions&,
      apache::thrift::MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader>,
      RequestClientCallback::Ptr cb,
      std::unique_ptr<folly::IOBuf>) override {
    static ClientReceiveState staticState = [&] {
      ClientReceiveState state;
      folly::Baton<> baton;
      auto client = makeTestClient<TestServiceAsyncClient>(
          std::make_shared<TestServiceSvNull>());
      client->echoInt(std::make_unique<DummyCallback>(state, baton), 0);
      baton.wait();
      state.serializedResponse().buffer->coalesce();
      return state;
    }();
    cb.release()->onResponse(ClientReceiveState(
        staticState.protocolId(),
        staticState.messageType(),
        SerializedResponse(staticState.serializedResponse().buffer->clone()),
        nullptr,
        nullptr,
        RpcTransportStats()));
  }
  void sendRequestNoResponse(
      const RpcOptions&,
      apache::thrift::MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader>,
      RequestClientCallback::Ptr cb,
      std::unique_ptr<folly::IOBuf>) override {
    cb.release()->onResponse({});
  }

  void setCloseCallback(CloseCallback*) override { std::terminate(); }

  folly::EventBase* getEventBase() const override { return nullptr; }

  uint16_t getProtocolId() override {
    return 2; // COMPACT
  }
};

class MockClient : public TestServiceAsyncClient {
 public:
  using TestServiceAsyncClient::TestServiceAsyncClient;

  template <int = 0>
  folly::coro::Task<int> co_old_echoInt(int x) {
    auto task = semifuture_echoInt(x);
    const folly::CancellationToken& cancelToken =
        co_await folly::coro::co_current_cancellation_token;
    if (cancelToken.canBeCancelled()) {
      co_yield folly::coro::co_result(
          co_await folly::coro::co_awaitTry(
              folly::coro::detachOnCancel(std::move(task))));
    } else {
      co_yield folly::coro::co_result(
          co_await folly::coro::co_awaitTry(std::move(task)));
    }
  }

  template <int = 0>
  folly::coro::Task<void> co_old_noResponse(int x) {
    auto task = semifuture_noResponse(x);
    const folly::CancellationToken& cancelToken =
        co_await folly::coro::co_current_cancellation_token;
    if (cancelToken.canBeCancelled()) {
      co_yield folly::coro::co_result(
          co_await folly::coro::co_awaitTry(
              folly::coro::detachOnCancel(std::move(task))));
    } else {
      co_yield folly::coro::co_result(
          co_await folly::coro::co_awaitTry(std::move(task)));
    }
  }
};

enum class Flavor {
  Sync,
  Future,
  Semifuture,
  Co,
  Co_old,
  Co_cancellable,
  Co_cancellable_old,
};

template <Flavor flavor, bool oneway>
void bench(size_t iters) {
  folly::BenchmarkSuspender susp;
  MockClient client(std::make_shared<DummyChannel>());
  if (!oneway) {
    client.sync_echoInt(0); // prime DummyChannel
  }
  folly::CancellationSource cs;
  susp.dismiss();
  while (iters--) {
    if (!oneway) {
      switch (flavor) {
        case Flavor::Sync:
          client.sync_echoInt(0);
          break;
        case Flavor::Future:
          client.future_echoInt(0).get();
          break;
        case Flavor::Semifuture:
          client.semifuture_echoInt(0).get();
          break;
        case Flavor::Co:
          folly::coro::blockingWait(client.co_echoInt(0));
          break;
        case Flavor::Co_old:
          folly::coro::blockingWait(client.co_old_echoInt(0));
          break;
        case Flavor::Co_cancellable:
          folly::coro::blockingWait(
              folly::coro::co_withCancellation(
                  cs.getToken(), client.co_echoInt(0)));
          break;
        case Flavor::Co_cancellable_old:
          folly::coro::blockingWait(
              folly::coro::co_withCancellation(
                  cs.getToken(), client.co_old_echoInt(0)));
          break;
      }
    } else {
      switch (flavor) {
        case Flavor::Sync:
          client.sync_noResponse(0);
          break;
        case Flavor::Future:
          client.future_noResponse(0).get();
          break;
        case Flavor::Semifuture:
          client.semifuture_noResponse(0).get();
          break;
        case Flavor::Co:
          folly::coro::blockingWait(client.co_noResponse(0));
          break;
        case Flavor::Co_old:
          folly::coro::blockingWait(client.co_old_noResponse(0));
          break;
        case Flavor::Co_cancellable:
          folly::coro::blockingWait(
              folly::coro::co_withCancellation(
                  cs.getToken(), client.co_noResponse(0)));
          break;
        case Flavor::Co_cancellable_old:
          folly::coro::blockingWait(
              folly::coro::co_withCancellation(
                  cs.getToken(), client.co_old_noResponse(0)));
          break;
      }
    }
  }
  susp.rehire();
}

#define NAME_true _oneway_
#define NAME_false _
#define ONEWAY(x) NAME_##x
#define CAT(x, y) x##y
#define EVALUATOR(x, y) CAT(x, y)
#define FLAVOR(flavor, oneway) EVALUATOR(flavor, ONEWAY(oneway))

#define X1(flavor, oneway)                                            \
  BENCHMARK(EVALUATOR(FLAVOR(flavor, oneway), Client_Bench), iters) { \
    bench<Flavor::flavor, oneway>(iters);                             \
  }

#define X2(oneway)           \
  X1(Sync, oneway)           \
  X1(Future, oneway)         \
  X1(Semifuture, oneway)     \
  X1(Co, oneway)             \
  X1(Co_old, oneway)         \
  X1(Co_cancellable, oneway) \
  X1(Co_cancellable_old, oneway)

X2(false)
X2(true)

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
