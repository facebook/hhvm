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
#include <folly/container/F14Set.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/bench/runner/ServerInterface.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

#include <memory>

namespace apache::thrift::fast_thrift::bench {

using TcpConnectionManager =
    apache::thrift::fast_thrift::rocket::server::connection::ConnectionManager;

/**
 * TcpServer owns all server infrastructure for TCP-based benchmarks.
 */
class TcpServer : public Server {
 public:
  explicit TcpServer(
      folly::SocketAddress address,
      uint32_t numIOThreads,
      size_t zeroCopyThreshold = 0);
  ~TcpServer() override;

  void start() override;
  void stop() override;

  folly::SocketAddress getAddress() const override;

 private:
  class ServerAppAdapter;

  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  TcpConnectionManager::Ptr connectionManager_;
  apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator
      allocator_;
  folly::F14FastSet<std::unique_ptr<ServerAppAdapter>> adapters_;
  size_t zeroCopyThreshold_{0};
};

} // namespace apache::thrift::fast_thrift::bench
