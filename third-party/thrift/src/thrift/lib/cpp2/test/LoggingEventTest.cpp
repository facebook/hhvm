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

#include <thrift/lib/cpp2/server/LoggingEvent.h>

#include <memory>
#include <unordered_map>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ServerInstrumentation.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

namespace {

using apache::thrift::ApplicationEventHandler;
using apache::thrift::ConnectionEventHandler;
using apache::thrift::ConnectionLoggingContext;
using apache::thrift::LoggingEventRegistry;
using apache::thrift::RequestEventHandler;
using apache::thrift::ServerEventHandler;
using apache::thrift::ServerTrackerHandler;
using apache::thrift::ThriftServer;
using apache::thrift::instrumentation::ServerTracker;

constexpr std::string_view kServe = "serve";
constexpr std::string_view kResourcePoolsEnabled = "resourcepoolsenabled";
constexpr std::string_view kDcheck = "dcheck";
// Note not setting a ssl config is seen as a manual override
constexpr std::string_view kNonTls = "non_tls.manual_policy";
constexpr std::string_view kNewConnection = "new_connection";
constexpr std::string_view kNewConnectionRocket = "new_connection.rocket";
constexpr std::string_view kNewConnectionHeader = "new_connection.header";
constexpr std::string_view kRocketSetup = "rocket.setup";
constexpr std::string_view kTransportMetadata = "transport.metadata";

using namespace apache::thrift;

class TestServerEventHandler : public ServerEventHandler {
 public:
  MOCK_METHOD(
      void, log, (const ThriftServer&, DynamicFieldsCallback), (override));
};

class TestConnectionEventHandler : public ConnectionEventHandler {
 public:
  MOCK_METHOD(
      void,
      log,
      (const ConnectionLoggingContext&, DynamicFieldsCallback),
      (override));
};

class TestServerTrackerHandler : public ServerTrackerHandler {
 public:
  MOCK_METHOD(void, log, (const ServerTracker&), (override));
};

class TestEventRegistry : public LoggingEventRegistry {
 public:
  TestEventRegistry() {
    serverEventMap_[kServe] = makeHandler<TestServerEventHandler>();
    serverEventMap_[kResourcePoolsEnabled] =
        makeHandler<TestServerEventHandler>();
    serverEventMap_[kDcheck] = makeHandler<TestServerEventHandler>();
    connectionEventMap_[kNonTls] = makeHandler<TestConnectionEventHandler>();
    connectionEventMap_[kNewConnection] =
        makeHandler<TestConnectionEventHandler>();
    connectionEventMap_[kNewConnectionRocket] =
        makeHandler<TestConnectionEventHandler>();
    connectionEventMap_[kNewConnectionHeader] =
        makeHandler<TestConnectionEventHandler>();
    connectionEventMap_[kRocketSetup] =
        makeHandler<TestConnectionEventHandler>();
    connectionEventMap_[kTransportMetadata] =
        makeHandler<TestConnectionEventHandler>();
    serverTrackerMap_
        [apache::thrift::instrumentation::kThriftServerTrackerKey] =
            makeHandler<TestServerTrackerHandler>();
  }

  ServerEventHandler& getServerEventHandler(
      std::string_view key) const override {
    return *serverEventMap_.at(key).get();
  }

  ConnectionEventHandler& getConnectionEventHandler(
      std::string_view key) const override {
    return *connectionEventMap_.at(key).get();
  }

  ApplicationEventHandler& getApplicationEventHandler(
      std::string_view /* key */) const override {
    static auto* handler = new ApplicationEventHandler();
    return *handler;
  }

  ServerTrackerHandler& getServerTrackerHandler(
      std::string_view key) const override {
    return *serverTrackerMap_.at(key).get();
  }

  RequestEventHandler& getRequestEventHandler(
      std::string_view /* key */) const override {
    static auto* handler = new RequestEventHandler();
    return *handler;
  }

 private:
  template <typename T>
  std::unique_ptr<T> makeHandler() {
    auto obj = std::make_unique<T>();
    testing::Mock::AllowLeak(obj.get());
    return obj;
  }

  std::unordered_map<std::string_view, std::unique_ptr<ServerEventHandler>>
      serverEventMap_;
  std::unordered_map<std::string_view, std::unique_ptr<ConnectionEventHandler>>
      connectionEventMap_;
  std::unordered_map<std::string_view, std::unique_ptr<ServerTrackerHandler>>
      serverTrackerMap_;
};
} // namespace

namespace apache::thrift::detail {
THRIFT_PLUGGABLE_FUNC_SET(
    std::unique_ptr<apache::thrift::LoggingEventRegistry>,
    makeLoggingEventRegistry) {
  return std::make_unique<TestEventRegistry>();
}
} // namespace apache::thrift::detail

namespace {
enum class TransportType { Header, Rocket };
} // namespace

class HeaderOrRocketTest {
 public:
  TransportType transport = TransportType::Rocket;

  bool isRocket() { return transport == TransportType::Rocket; }

  template <typename ClientT>
  auto makeClient(ScopedServerInterfaceThread& runner) {
    if (transport == TransportType::Header) {
      return runner.newClient<ClientT>(nullptr, [&](auto socket) mutable {
        return HeaderClientChannel::newChannel(std::move(socket));
      });
    } else {
      return runner.newClient<ClientT>(nullptr, [&](auto socket) mutable {
        return RocketClientChannel::newChannel(std::move(socket));
      });
    }
  }
};

template <typename T>
class LoggingEventTest : public testing::Test {
 protected:
  template <typename H>
  T& fetchHandler(
      H& (LoggingEventRegistry::*method)(std::string_view) const,
      std::string_view key) {
    auto& handler = handlers_[key];
    if (!handler) {
      handler = dynamic_cast<T*>(&(getLoggingEventRegistry().*method)(key));
      EXPECT_NE(handler, nullptr);
    }
    return *handler;
  }

  void TearDown() override {
    for (auto& h : handlers_) {
      ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(h.second));
    }
  }

 private:
  std::map<std::string_view, T*> handlers_;
};

class ServerEventLogTest : public LoggingEventTest<TestServerEventHandler> {
 protected:
  void expectServerEventCall(std::string_view key, size_t times) {
    auto& handler =
        fetchHandler(&LoggingEventRegistry::getServerEventHandler, key);
    EXPECT_CALL(handler, log(testing::_, testing::_)).Times(times);
  }
};

class ServerTrackerLogTest : public LoggingEventTest<TestServerTrackerHandler> {
 protected:
  void expectServerTrackerCall(std::string_view key, size_t times) {
    auto& handler =
        fetchHandler(&LoggingEventRegistry::getServerTrackerHandler, key);
    EXPECT_CALL(handler, log(testing::_)).Times(times);
  }
};

class TestServiceHandler
    : public apache::thrift::ServiceHandler<apache::thrift::test::TestService> {
 public:
  void voidResponse() override {}
};

TEST_F(ServerEventLogTest, serverTest) {
  expectServerEventCall(kServe, 1);
  if (apache::thrift::useResourcePoolsFlagsSet()) {
    expectServerEventCall(kResourcePoolsEnabled, 1);
  }
  auto handler = std::make_shared<TestServiceHandler>();
  apache::thrift::ScopedServerInterfaceThread server(handler);
}

TEST_F(ServerTrackerLogTest, serverTest) {
  expectServerTrackerCall(
      apache::thrift::instrumentation::kThriftServerTrackerKey, 1);
  auto handler = std::make_shared<TestServiceHandler>();
  apache::thrift::ScopedServerInterfaceThread server(handler);
}

class ConnectionEventLogTest
    : public LoggingEventTest<TestConnectionEventHandler>,
      public HeaderOrRocketTest,
      public ::testing::WithParamInterface<TransportType> {
 public:
  void SetUp() override { transport = GetParam(); }
  void expectConnectionEventCall(std::string_view key, size_t times) {
    auto& handler =
        fetchHandler(&LoggingEventRegistry::getConnectionEventHandler, key);
    EXPECT_CALL(handler, log(testing::_, testing::_)).Times(times);
  }
};

TEST_P(ConnectionEventLogTest, connectionTest) {
  expectConnectionEventCall(kNonTls, 1);
  expectConnectionEventCall(kNewConnection, 1);
  expectConnectionEventCall(kNewConnectionRocket, isRocket() ? 1 : 0);
  expectConnectionEventCall(kNewConnectionHeader, isRocket() ? 0 : 1);
  expectConnectionEventCall(kRocketSetup, isRocket() ? 1 : 0);
  auto handler = std::make_shared<TestServiceHandler>();
  ScopedServerInterfaceThread runner(handler);
  auto client = makeClient<test::TestServiceAsyncClient>(runner);

  // block to make sure request is actually sent.
  client->semifuture_voidResponse().get();
}

INSTANTIATE_TEST_CASE_P(
    HeaderOrRocket,
    ConnectionEventLogTest,
    testing::Values(TransportType::Header, TransportType::Rocket));
