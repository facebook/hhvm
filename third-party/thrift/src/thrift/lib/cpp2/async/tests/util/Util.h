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

#include <folly/SocketAddress.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp/server/TServerEventHandler.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/async/tests/util/TestSinkService.h>
#include <thrift/lib/cpp2/async/tests/util/TestStreamService.h>
#include <thrift/lib/cpp2/async/tests/util/gen-cpp2/TestBiDiService.h>
#include <thrift/lib/cpp2/async/tests/util/gen-cpp2/TestSinkServiceAsyncClient.h>
#include <thrift/lib/cpp2/async/tests/util/gen-cpp2/TestStreamServiceAsyncClient.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketRoutingHandler.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/TestUtil.h>

namespace apache::thrift {

template <class Handler, class Client>
class AsyncTestSetup : public TestSetup {
 protected:
  void SetUp() override {
    handler_ = std::make_shared<Handler>();
    setNumIOThreads(numIOThreads_);
    setNumWorkerThreads(numWorkerThreads_);
    setQueueTimeout(std::chrono::milliseconds(0));
    setIdleTimeout(std::chrono::milliseconds(0));
    setTaskExpireTime(std::chrono::milliseconds(0));
    setStreamExpireTime(std::chrono::milliseconds(0));
    server_ = createServer(handler_, serverPort_);
  }

  void TearDown() override {
    if (server_) {
      server_->cleanUp();
      server_.reset();
      handler_.reset();
    }
  }

  template <class SocketT = folly::AsyncSocket>
  void connectToServer(
      folly::Function<folly::coro::Task<void>(Client&)> callMe,
      std::chrono::milliseconds timeout = std::chrono::milliseconds{500}) {
    folly::coro::blockingWait(
        [this, &callMe, &timeout]() -> folly::coro::Task<void> {
          CHECK_GT(serverPort_, 0) << "Check if the server has started already";
          folly::Executor* executor = co_await folly::coro::co_current_executor;
          auto channel = PooledRequestChannel::newChannel(
              executor, ioThread_, [&](folly::EventBase& evb) {
                auto rocketChannel =
                    apache::thrift::RocketClientChannel::newChannel(
                        folly::AsyncSocket::UniquePtr(
                            new SocketT(&evb, "::1", serverPort_)));
                rocketChannel->setTimeout(timeout.count() /* timeoutMs */);
                return rocketChannel;
              });
          Client client(std::move(channel));
          co_await callMe(client);
        }());
  }

 protected:
  int numIOThreads_{1};
  int numWorkerThreads_{1};
  uint16_t serverPort_{0};
  std::shared_ptr<folly::IOExecutor> ioThread_{
      std::make_shared<folly::ScopedEventBaseThread>()};
  std::unique_ptr<ThriftServer> server_;
  std::shared_ptr<Handler> handler_;
};

class DuplicateWriteSocket : public folly::AsyncSocket {
 public:
  using folly::AsyncSocket::AsyncSocket;

  void writeChain(
      WriteCallback* callback,
      std::unique_ptr<IOBuf>&& buf,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override {
    // first request sends setup frame, don't duplicate this payload
    if (firstWrite_) {
      firstWrite_ = false;
    } else {
      buf->appendChain(buf->clone());
    }
    folly::AsyncSocket::writeChain(callback, std::move(buf), flags);
  }

 private:
  bool firstWrite_{true};
};

} // namespace apache::thrift
