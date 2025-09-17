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

#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <thrift/example/cpp2/server/EchoService.h>
#include <thrift/example/if/gen-cpp2/Echo.h>
#include <thrift/perf/cpp2/util/Util.h>

DEFINE_string(host, "::1", "EchoServer host");
DEFINE_int32(port, 7778, "EchoServer port");
DEFINE_string(
    transport,
    "header",
    "Transport to use: header, rsocket, http2, or inmemory");

using example::chatroom::EchoAsyncClient;

int main(int argc, char* argv[]) {
  FLAGS_logtostderr = true;
  const folly::Init init(&argc, &argv);

  // create eventbase first so no dangling stack refs on 'client' dealloc
  folly::EventBase evb;

  // Create a thrift client
  auto addr = folly::SocketAddress(FLAGS_host, FLAGS_port);
  auto ct = std::make_shared<ConnectionThread<EchoAsyncClient>>();
  auto client = ct->newSyncClient(addr, FLAGS_transport);

  // For header transport
  if (FLAGS_transport == "header") {
    client = newHeaderClient<EchoAsyncClient>(&evb, addr);
  }

  // Prepare thrift request
  std::string message = "Ping this back";
  std::string response;

  // Get an echo'd message
  try {
    client->sync_echo(response, message);
    LOG(INFO) << response;
  } catch (apache::thrift::transport::TTransportException& ex) {
    LOG(ERROR) << "Request failed " << ex.what();
  }

  return 0;
}
