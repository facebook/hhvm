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

#include <chrono>
#include <mutex>

#include <folly/SharedMutex.h>
#include <folly/Synchronized.h>
#include <folly/experimental/coro/Baton.h>
#include <folly/experimental/coro/GtestHelpers.h>
#include <folly/experimental/observer/Observer.h>
#include <folly/experimental/observer/SimpleObservable.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/DummyStatus.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace std::literals;

using apache::thrift::PolledServiceHealth;
using apache::thrift::ScopedServerInterfaceThread;
using apache::thrift::ThriftServer;
using ServiceHealth = PolledServiceHealth::ServiceHealth;
using apache::thrift::test::DummyStatus;
using apache::thrift::test::DummyStatusAsyncClient;

namespace {
class Handler : public apache::thrift::ServiceHandler<DummyStatus>,
                public PolledServiceHealth {
 public:
  explicit Handler(ServiceHealth serviceHealth)
      : serviceHealth_(serviceHealth) {}

  folly::coro::Task<ServiceHealth> co_getServiceHealth() override {
    folly::SharedMutex::ReadHolder guard{healthMutex_};
    // ensure the baton is not reset() while posting
    polled_.lock()->post();
    co_return **serviceHealthObserver_;
  }

  void async_eb_getStatus(
      std::unique_ptr<apache::thrift::HandlerCallback<std::int64_t>> callback)
      override {
    ThriftServer* server = callback->getRequestContext()
                               ->getConnectionContext()
                               ->getWorker()
                               ->getServer();
    callback->result(static_cast<std::int64_t>(
        server->getServiceHealth().value_or(ServiceHealth{})));
  }

  folly::coro::Task<void> co_waitForPoll() {
    polled_.lock()->reset();
    // waiting and posting a baton concurrently is safe
    co_await polled_.unsafeGetUnlocked();
  }

  void setHealth(ServiceHealth value) {
    std::unique_lock guard{healthMutex_};
    serviceHealth_.setValue(value);
    folly::observer_detail::ObserverManager::waitForAllUpdates();
  }

 private:
  folly::observer::SimpleObservable<ServiceHealth> serviceHealth_;
  folly::observer::Observer<ServiceHealth> serviceHealthObserver_{
      serviceHealth_.getObserver()};
  folly::SharedMutex healthMutex_;
  folly::Synchronized<folly::coro::Baton, std::mutex> polled_;
};

std::int64_t i64(ServiceHealth value) {
  return static_cast<std::int64_t>(value);
}
} // namespace

CO_TEST(ServiceHealthPollingTest, Basic) {
  constexpr static auto kLiveness = 100ms;

  auto handler = std::make_shared<Handler>(ServiceHealth::OK);
  ScopedServerInterfaceThread runner{
      handler, [](ThriftServer& server) {
        server.setPolledServiceHealthLiveness(kLiveness);
      }};

  co_await handler->co_waitForPoll();
  auto client = runner.newClient<DummyStatusAsyncClient>();
  EXPECT_EQ(co_await client->co_getStatus(), i64(ServiceHealth::OK));

  co_await handler->co_waitForPoll();
  handler->setHealth(ServiceHealth::ERROR);
  EXPECT_EQ(co_await client->co_getStatus(), i64(ServiceHealth::OK));

  co_await handler->co_waitForPoll();
  EXPECT_EQ(co_await client->co_getStatus(), i64(ServiceHealth::ERROR));

  co_await handler->co_waitForPoll();
  handler->setHealth(ServiceHealth::OK);
  EXPECT_EQ(co_await client->co_getStatus(), i64(ServiceHealth::ERROR));

  co_await handler->co_waitForPoll();
  EXPECT_EQ(co_await client->co_getStatus(), i64(ServiceHealth::OK));
}
