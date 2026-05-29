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
#include <string>

namespace apache::thrift::fast_thrift::bench {

class TcpClient;

/**
 * Factory for creating concrete Client instances based on test type.
 */
class ClientFactory {
 public:
  explicit ClientFactory(
      const std::string& testType, size_t zeroCopyThreshold = 0)
      : testType_(testType), zeroCopyThreshold_(zeroCopyThreshold) {}

  std::unique_ptr<Client> createClient(
      folly::EventBase* evb,
      const folly::SocketAddress& addr,
      folly::AsyncSocket::ConnectCallback* cb);

 private:
  std::unique_ptr<TcpClient> createTcpClient(
      folly::EventBase* evb,
      const folly::SocketAddress& addr,
      folly::AsyncSocket::ConnectCallback* cb);

  std::string testType_;
  size_t zeroCopyThreshold_{0};
};

} // namespace apache::thrift::fast_thrift::bench
