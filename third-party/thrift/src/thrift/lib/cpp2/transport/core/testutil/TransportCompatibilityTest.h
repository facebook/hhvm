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

#pragma once

#include <gmock/gmock.h>

#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/TransportRoutingHandler.h>
#include <thrift/lib/cpp2/transport/core/ClientConnectionIf.h>
#include <thrift/lib/cpp2/transport/core/testutil/FakeServerObserver.h>
#include <thrift/lib/cpp2/transport/core/testutil/TestServiceMock.h>

namespace apache::thrift {

template <typename Service>
class SampleServer {
 public:
  explicit SampleServer(std::shared_ptr<Service> handler);
  virtual ~SampleServer();

  SampleServer(const SampleServer&) = delete;
  SampleServer& operator=(const SampleServer&) = delete;
  SampleServer(SampleServer&&) = default;
  SampleServer& operator=(SampleServer&&) = default;

  // Don't forget to start the server before running the tests.
  void startServer();

  void addRoutingHandler(
      std::unique_ptr<TransportRoutingHandler> routingHandler);
  ThriftServer* getServer();

  void connectToServer(
      std::string transport,
      bool withUpgrade,
      folly::Function<void(
          std::shared_ptr<RequestChannel>, std::shared_ptr<ClientConnectionIf>)>
          callMe);

 protected:
  void setupServer();
  void stopServer();

 public:
  std::shared_ptr<FakeServerObserver> observer_;

 protected:
  std::shared_ptr<Service> handler_;
  std::unique_ptr<ThriftServer> server_;
  uint16_t port_;

  int numIOThreads_{10};
  int numWorkerThreads_{10};

  folly::ScopedEventBaseThread evbThread_;

  friend class TransportCompatibilityTest;
};

// Transport layer compliance tests.
class TransportCompatibilityTest {
 public:
  TransportCompatibilityTest();

  // Don't forget to start the server before running the tests.
  void startServer();

  void addRoutingHandler(
      std::unique_ptr<TransportRoutingHandler> routingHandler);

  void setTransportUpgradeExpected(bool upgradeToRocketExpected) {
    upgradeToRocketExpected_ = upgradeToRocketExpected;
  }

  ThriftServer* getServer();

  void connectToServer(
      folly::Function<
          void(std::unique_ptr<testutil::testservice::TestServiceAsyncClient>)>
          callMe);

  void TestRequestResponse_Simple();
  void TestRequestResponse_Sync();
  void TestRequestResponse_Destruction();
  void TestRequestResponse_MultipleClients();
  void TestRequestResponse_ExpectedException();
  void TestRequestResponse_UnexpectedException();
  void TestRequestResponse_Timeout();
  void TestRequestResponse_Header();
  void TestRequestResponse_Header_Load();
  void TestRequestResponse_Header_ExpectedException();
  void TestRequestResponse_Header_UnexpectedException();
  void TestRequestResponse_Saturation();
  void TestRequestResponse_IsOverloaded();
  void TestRequestResponse_Connection_CloseNow();
  void TestRequestResponse_ServerQueueTimeout();
  void TestRequestResponse_ResponseSizeTooBig();
  void TestRequestResponse_Checksumming();

  void TestOneway_Simple();
  void TestOneway_WithDelay();
  void TestOneway_Saturation();
  void TestOneway_UnexpectedException();
  void TestOneway_Connection_CloseNow();
  void TestOneway_ServerQueueTimeout();
  void TestOneway_Checksumming(bool usingSampling = false);

  void TestRequestContextIsPreserved();
  void TestBadPayload();
  void TestEvbSwitch();
  void TestEvbSwitch_Failure();
  void TestCloseCallback();

  void TestConnectionStats();
  void TestObserverSendReceiveRequests();
  void TestConnectionContext();
  void TestClientIdentityHook();

  void TestCustomAsyncProcessor();
  void TestOnWriteQuiescence();

 protected:
  void connectToServer(
      folly::Function<void(
          std::unique_ptr<testutil::testservice::TestServiceAsyncClient>,
          std::shared_ptr<ClientConnectionIf>)> callMe);

  void callSleep(
      testutil::testservice::TestServiceAsyncClient* client,
      int32_t timeoutMs,
      int32_t sleepMs);

 public:
  std::shared_ptr<testutil::testservice::TestServiceMock> handler_;

 protected:
  std::unique_ptr<SampleServer<testutil::testservice::TestServiceMock>> server_;
  bool upgradeToRocketExpected_{true};
};

} // namespace apache::thrift
