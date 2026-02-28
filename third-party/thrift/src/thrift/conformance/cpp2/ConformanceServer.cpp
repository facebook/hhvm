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

#include <iostream>
#include <memory>

#include <glog/logging.h>
#include <folly/init/Init.h>
#include <thrift/conformance/cpp2/AnyRegistry.h>
#include <thrift/conformance/cpp2/ConformanceHandler.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  auto server = std::make_shared<apache::thrift::ThriftServer>();
  auto handler =
      std::make_shared<apache::thrift::conformance::ConformanceHandler>();
  server->setInterface(handler);
  server->setPort(0); // Any free port.
  server->setup();
  LOG(INFO)
      << apache::thrift::conformance::AnyRegistry::generated().debugString();
  std::cout << server->getAddress().getPort() << std::endl;
  server->waitForStop();
  return 0;
}
