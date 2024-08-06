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

#include <folly/portability/GFlags.h>

#include <proxygen/httpserver/HTTPServerOptions.h>
#include <thrift/lib/cpp2/transport/core/testutil/TransportCompatibilityTest.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>
#include <thrift/lib/cpp2/transport/util/ConnectionThread.h>

namespace apache {
namespace thrift {

std::unique_ptr<HTTP2RoutingHandler> createHTTP2RoutingHandler(
    ThriftServer* server) {
  auto h2_options = std::make_unique<proxygen::HTTPServerOptions>();
  h2_options->threads = static_cast<size_t>(server->getNumIOWorkerThreads());
  h2_options->idleTimeout = server->getIdleTimeout();
  h2_options->shutdownOn = {SIGINT, SIGTERM};

  return std::make_unique<HTTP2RoutingHandler>(
      std::move(h2_options), server->getThriftProcessor(), *server);
}

class H2CompatibilityTest : public testing::Test {
 public:
  H2CompatibilityTest() {
    FLAGS_transport = "http2"; // client's transport

    compatibilityTest_ = std::make_unique<TransportCompatibilityTest>();
    // HTTP transport is not expected to upgrade to Rocket
    compatibilityTest_->setTransportUpgradeExpected(false);
    compatibilityTest_->addRoutingHandler(
        createHTTP2RoutingHandler(compatibilityTest_->getServer()));
    compatibilityTest_->startServer();
  }

 protected:
  std::unique_ptr<TransportCompatibilityTest> compatibilityTest_;
};

TEST_F(H2CompatibilityTest, RequestResponse_Simple) {
  compatibilityTest_->TestRequestResponse_Simple();
}

TEST_F(H2CompatibilityTest, RequestResponse_Sync) {
  compatibilityTest_->TestRequestResponse_Sync();
}

TEST_F(H2CompatibilityTest, RequestResponse_MultipleClients) {
  compatibilityTest_->TestRequestResponse_MultipleClients();
}

TEST_F(H2CompatibilityTest, RequestResponse_ExpectedException) {
  compatibilityTest_->TestRequestResponse_ExpectedException();
}

TEST_F(H2CompatibilityTest, RequestResponse_UnexpectedException) {
  compatibilityTest_->TestRequestResponse_UnexpectedException();
}

// Warning: This test may be flaky due to use of timeouts.
TEST_F(H2CompatibilityTest, RequestResponse_Timeout) {
  compatibilityTest_->TestRequestResponse_Timeout();
}

TEST_F(H2CompatibilityTest, RequestResponse_Header) {
  compatibilityTest_->TestRequestResponse_Header();
}

TEST_F(H2CompatibilityTest, RequestResponse_Header_ExpectedException) {
  compatibilityTest_->TestRequestResponse_Header_ExpectedException();
}

TEST_F(H2CompatibilityTest, RequestResponse_Header_UnexpectedException) {
  compatibilityTest_->TestRequestResponse_Header_UnexpectedException();
}

TEST_F(H2CompatibilityTest, RequestResponse_Saturation) {
  compatibilityTest_->TestRequestResponse_Saturation();
}

TEST_F(H2CompatibilityTest, RequestResponse_Connection_CloseNow) {
  compatibilityTest_->TestRequestResponse_Connection_CloseNow();
}

TEST_F(H2CompatibilityTest, RequestResponse_ServerQueueTimeout) {
  compatibilityTest_->TestRequestResponse_ServerQueueTimeout();
}

TEST_F(H2CompatibilityTest, RequestResponse_ResponseSizeTooBig) {
  compatibilityTest_->TestRequestResponse_ResponseSizeTooBig();
}

TEST_F(H2CompatibilityTest, Oneway_Simple) {
  compatibilityTest_->TestOneway_Simple();
}

TEST_F(H2CompatibilityTest, Oneway_WithDelay) {
  compatibilityTest_->TestOneway_WithDelay();
}

TEST_F(H2CompatibilityTest, Oneway_Saturation) {
  compatibilityTest_->TestOneway_Saturation();
}

TEST_F(H2CompatibilityTest, Oneway_UnexpectedException) {
  compatibilityTest_->TestOneway_UnexpectedException();
}

TEST_F(H2CompatibilityTest, Oneway_Connection_CloseNow) {
  compatibilityTest_->TestOneway_Connection_CloseNow();
}

TEST_F(H2CompatibilityTest, Oneway_ServerQueueTimeout) {
  compatibilityTest_->TestOneway_ServerQueueTimeout();
}

TEST_F(H2CompatibilityTest, RequestContextIsPreserved) {
  compatibilityTest_->TestRequestContextIsPreserved();
}

TEST_F(H2CompatibilityTest, EvbSwitch) {
  compatibilityTest_->TestEvbSwitch();
}

TEST_F(H2CompatibilityTest, EvbSwitch_Failure) {
  compatibilityTest_->TestEvbSwitch_Failure();
}

TEST_F(H2CompatibilityTest, CloseCallback) {
  compatibilityTest_->TestCloseCallback();
}

TEST_F(H2CompatibilityTest, ConnectionStats) {
  compatibilityTest_->TestConnectionStats();
}

TEST_F(H2CompatibilityTest, ObserverSendReceiveRequests) {
  compatibilityTest_->TestObserverSendReceiveRequests();
}

} // namespace thrift
} // namespace apache
