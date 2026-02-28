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

namespace apache::thrift {

std::unique_ptr<HTTP2RoutingHandler> createHTTP2RoutingHandler(
    ThriftServer* server) {
  auto h2_options = std::make_unique<proxygen::HTTPServerOptions>();
  h2_options->threads = static_cast<size_t>(server->getNumIOWorkerThreads());
  h2_options->idleTimeout = server->getIdleTimeout();
  h2_options->shutdownOn = {SIGINT, SIGTERM};

  return std::make_unique<HTTP2RoutingHandler>(
      std::move(h2_options), server->getThriftProcessor(), *server);
}

class LegacyCompatibilityTest : public testing::Test {
 public:
  LegacyCompatibilityTest() {
    FLAGS_transport = "legacy-http2"; // client's transport

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

TEST_F(LegacyCompatibilityTest, RequestResponse_Sync) {
  compatibilityTest_->TestRequestResponse_Sync();
}

TEST_F(LegacyCompatibilityTest, RequestResponse_ResponseSizeTooBig) {
  compatibilityTest_->TestRequestResponse_ResponseSizeTooBig();
}

} // namespace apache::thrift
