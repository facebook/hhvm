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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

namespace apache::thrift::test {

/**
 * Shared test fixture for Thrift RPC E2E tests.
 *
 * Provides helpers to spin up an in-process server and create typed clients.
 * Each test defines its handler struct inline and calls testConfig() to
 * configure the server, then makeClient<ServiceTag>() to obtain a client.
 */
class E2ETestFixture : public ::testing::Test {
  using MakeChannelFunc = ScopedServerInterfaceThread::MakeChannelFunc;

 public:
  struct TestConfig {
    std::shared_ptr<AsyncProcessorFactory> handler;
    MakeChannelFunc channelFunc =
        [](folly::AsyncSocket::UniquePtr socket) -> RequestChannel::Ptr {
      return RocketClientChannel::newChannel(std::move(socket));
    };
  };

  void testConfig(TestConfig&& config) {
    server_ = std::make_unique<ScopedServerInterfaceThread>(
        std::move(config.handler));
    channelFunc_ = std::move(config.channelFunc);
  }

  template <typename ServiceTag>
  std::unique_ptr<Client<ServiceTag>> makeClient() {
    return server_->newClient<Client<ServiceTag>>(
        /* callbackExecutor */ nullptr,
        [&](folly::AsyncSocket::UniquePtr socket) -> RequestChannel::Ptr {
          return channelFunc_(std::move(socket));
        });
  }

 private:
  std::unique_ptr<ScopedServerInterfaceThread> server_;
  MakeChannelFunc channelFunc_;
};

} // namespace apache::thrift::test
