/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thread>

#include <folly/init/Init.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/test/loadshedding/server/BackendServiceHandler.h>

using namespace apache::thrift;
using namespace facebook::thrift::test;
using namespace std::chrono;

DEFINE_int32(port, 7777, "Port for the thrift server");

int main(int argc, char** argv) {
  folly::init(&argc, &argv);

  auto handler = std::make_shared<BackendServiceHandler>();
  auto server = std::make_shared<ThriftServer>();
  server->setPort(FLAGS_port);
  server->setInterface(handler);

  std::thread logger([&] {
    auto last = std::chrono::system_clock::now();
    for (;;) {
      auto sleepTime = std::chrono::seconds(1);
      std::this_thread::sleep_for(sleepTime);
      auto now = std::chrono::system_clock::now();
      auto elapsed =
          std::chrono::duration_cast<std::chrono::seconds>(now - last);
      last = now;
      double counter = handler->getAndResetRequestCount();
      LOG(INFO) << "RPS: " << (counter / elapsed.count())
                << ", active req: " << server->getActiveRequests();
    }
  });

  LOG(INFO) << "BackendService running on port: " << FLAGS_port;
  server->serve();

  return 0;
}
