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

#include <chrono>
#include <memory>

#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/util/TestServerFactory.h>

namespace apache {
namespace thrift {

template <typename Interface>
struct TestThriftServerFactory : public TestServerFactory {
 public:
  std::shared_ptr<BaseThriftServer> create() override {
    auto server = std::make_shared<apache::thrift::ThriftServer>();
    server->setNumIOWorkerThreads(1);
    if (useSimpleThreadManager_) {
      auto threadFactory =
          std::make_shared<apache::thrift::concurrency::PosixThreadFactory>();
      server->setThreadManagerType(
          apache::thrift::BaseThriftServer::ThreadManagerType::SIMPLE);
      server->setNumCPUWorkerThreads(1);
      server->setThreadFactory(threadFactory);
    } else if (setupFunction_) {
      setupFunction_(*server);
    }

    server->setPort(0);

    if (idleTimeoutMs_ != 0) {
      server->setIdleTimeout(std::chrono::milliseconds(idleTimeoutMs_));
    }

    if (serverEventHandler_) {
      server->setServerEventHandler(serverEventHandler_);
    }

    server->setInterface(std::unique_ptr<Interface>(new Interface));
    return server;
  }

  TestThriftServerFactory& useSimpleThreadManager(bool use) override {
    useSimpleThreadManager_ = use;
    return *this;
  }

  TestThriftServerFactory& setServerSetupFunction(
      std::function<void(BaseThriftServer&)> setupFunction) override {
    setupFunction_ = setupFunction;
    return *this;
  }

  TestThriftServerFactory& idleTimeoutMs(uint32_t idle) {
    idleTimeoutMs_ = idle;
    return *this;
  }

 private:
  bool useSimpleThreadManager_{true};
  std::function<void(BaseThriftServer&)> setupFunction_;
  uint32_t idleTimeoutMs_{0};
};

} // namespace thrift
} // namespace apache
