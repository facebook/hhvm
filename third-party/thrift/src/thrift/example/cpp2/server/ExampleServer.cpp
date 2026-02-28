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

#include <glog/logging.h>
#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>
#include <proxygen/httpserver/HTTPServerOptions.h>
#include <thrift/example/cpp2/server/ChatRoomService.h>
#include <thrift/example/cpp2/server/EchoService.h>
#include <thrift/lib/cpp2/server/ThriftProcessor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>

DEFINE_int32(chatroom_port, 7777, "Chatroom Server port");
DEFINE_int32(echo_port, 7778, "Echo Server port");

using apache::thrift::HTTP2RoutingHandler;
using apache::thrift::ThriftServer;
using example::chatroom::ChatRoomServiceHandler;
using example::chatroom::EchoHandler;
using proxygen::HTTPServerOptions;

std::unique_ptr<HTTP2RoutingHandler> createHTTP2RoutingHandler(
    std::shared_ptr<ThriftServer> server) {
  auto h2_options = std::make_unique<HTTPServerOptions>();
  h2_options->threads = static_cast<size_t>(server->getNumIOWorkerThreads());
  h2_options->idleTimeout = server->getIdleTimeout();
  h2_options->shutdownOn = {SIGINT, SIGTERM};
  return std::make_unique<HTTP2RoutingHandler>(
      std::move(h2_options), server->getThriftProcessor(), *server);
}

template <typename ServiceHandler>
std::shared_ptr<ThriftServer> newServer(int32_t port) {
  auto handler = std::make_shared<ServiceHandler>();
  auto server = std::make_shared<ThriftServer>();
  server->setPort(port);
  server->setInterface(handler);
  server->addRoutingHandler(createHTTP2RoutingHandler(server));
  return server;
}

int main(int argc, char** argv) {
  FLAGS_logtostderr = 1;
  const folly::Init init(&argc, &argv);

  auto chatroom_server = newServer<ChatRoomServiceHandler>(FLAGS_chatroom_port);
  std::thread t([&] {
    LOG(INFO) << "ChatRoom Server running on port: " << FLAGS_chatroom_port;
    chatroom_server->serve();
  });

  auto echo_server = newServer<EchoHandler>(FLAGS_echo_port);
  LOG(INFO) << "Echo Server running on port: " << FLAGS_echo_port;
  echo_server->serve();

  return 0;
}
