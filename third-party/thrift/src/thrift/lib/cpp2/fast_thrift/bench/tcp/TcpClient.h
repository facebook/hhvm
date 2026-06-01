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

#include <thrift/lib/cpp2/fast_thrift/bench/runner/ClientInterface.h>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>

#include <memory>

namespace apache::thrift::fast_thrift::bench {

/**
 * TcpClient wraps the raw channel pipeline for TCP-based benchmarks.
 *
 * This provides a clean interface for TCP-based benchmarks, encapsulating
 * the pipeline setup and connection management. The echo() method provides
 * a simple request-response API for benchmark tests.
 *
 * Usage:
 *   TcpClient client;
 *   client.connect(evb, serverAddress, connectCallback);
 *   auto resp = co_await client.echo(payload);
 */
class TcpClient : public Client {
 public:
  TcpClient();
  ~TcpClient() override;

  TcpClient(TcpClient&&) noexcept;
  TcpClient& operator=(TcpClient&&) noexcept;

  TcpClient(const TcpClient&) = delete;
  TcpClient& operator=(const TcpClient&) = delete;

  /**
   * Connect to the server. Must be called from the EventBase thread.
   */
  void connect(
      folly::EventBase* evb,
      const folly::SocketAddress& serverAddress,
      folly::AsyncSocket::ConnectCallback* connectCb,
      size_t zeroCopyThreshold = 0);

  folly::Future<std::unique_ptr<folly::IOBuf>> echo(
      std::unique_ptr<folly::IOBuf> data) override;

  void shutdown() override;

 private:
  class AppAdapter;
  std::unique_ptr<AppAdapter> appAdapter_;
};

} // namespace apache::thrift::fast_thrift::bench
