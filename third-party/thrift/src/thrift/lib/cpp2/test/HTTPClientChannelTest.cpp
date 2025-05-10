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

#include <thrift/lib/cpp2/async/HTTPClientChannel.h>

#include <gtest/gtest.h>
#include <folly/io/async/AsyncTransport.h>

#include <folly/io/async/AsyncSocket.h>
#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <wangle/channel/AsyncSocketHandler.h>

#include <boost/lexical_cast.hpp>

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::test;
using std::string;

class TestServiceHandler : public apache::thrift::ServiceHandler<TestService> {
 public:
  void sendResponse(string& _return, int64_t size) override {
    if (size >= 0) {
      usleep(size);
    }

    _return = "test" + boost::lexical_cast<std::string>(size);
  }
};

std::unique_ptr<HTTP2RoutingHandler> createHTTP2RoutingHandler(
    ThriftServer& server) {
  auto h2_options = std::make_unique<proxygen::HTTPServerOptions>();
  h2_options->threads = static_cast<size_t>(server.getNumIOWorkerThreads());
  h2_options->idleTimeout = server.getIdleTimeout();
  h2_options->shutdownOn = {SIGINT, SIGTERM};
  return std::make_unique<HTTP2RoutingHandler>(
      std::move(h2_options), server.getThriftProcessor(), server);
}

std::shared_ptr<ThriftServer> createHttpServer() {
  auto handler = std::make_shared<TestServiceHandler>();
  auto server = std::make_shared<ThriftServer>();
  server->setAddress({"::1", 0});
  server->setInterface(handler);
  server->setNumIOWorkerThreads(1);
  server->setThreadFactory(std::make_shared<PosixThreadFactory>());
  server->setThreadManagerType(ThriftServer::ThreadManagerType::SIMPLE);
  server->addRoutingHandler(createHTTP2RoutingHandler(*server));
  return server;
}

TEST(HTTPClientChannelTest, Basic) {
  ScopedServerInterfaceThread runner(createHttpServer());
  const auto addr = runner.getAddress();

  folly::EventBase eb;
  folly::AsyncTransport::UniquePtr socket(new folly::AsyncSocket(&eb, addr));

  TestServiceAsyncClient client(
      HTTPClientChannel::newHTTP2Channel(std::move(socket)));

  std::string ret;
  client.sync_sendResponse(ret, 42);
  EXPECT_EQ(ret, "test42");
}

TEST(HTTPClientChannelTest, NoGoodChannel) {
  ScopedServerInterfaceThread runner(createHttpServer());
  const auto addr = runner.getAddress();

  folly::EventBase eb;
  folly::AsyncTransport::UniquePtr socket(new folly::AsyncSocket(&eb, addr));

  auto channel = HTTPClientChannel::newHTTP2Channel(std::move(socket));

  EXPECT_TRUE(channel->good());

  channel->getTransport()->close();

  EXPECT_FALSE(channel->good());
}
