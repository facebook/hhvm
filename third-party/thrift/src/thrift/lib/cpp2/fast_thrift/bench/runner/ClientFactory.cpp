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

#include <thrift/lib/cpp2/fast_thrift/bench/runner/ClientFactory.h>

#include <thrift/lib/cpp2/fast_thrift/bench/tcp/TcpClient.h>

namespace apache::thrift::fast_thrift::bench {

std::unique_ptr<Client> ClientFactory::createClient(
    folly::EventBase* evb,
    const folly::SocketAddress& addr,
    folly::AsyncSocket::ConnectCallback* cb) {
  if (testType_ == "tcp") {
    return createTcpClient(evb, addr, cb);
  } else {
    LOG(FATAL) << "Unknown test type: " << testType_;
  }
}

std::unique_ptr<TcpClient> ClientFactory::createTcpClient(
    folly::EventBase* evb,
    const folly::SocketAddress& addr,
    folly::AsyncSocket::ConnectCallback* cb) {
  auto client = std::make_unique<TcpClient>();
  client->connect(evb, addr, cb, zeroCopyThreshold_);
  return client;
}

} // namespace apache::thrift::fast_thrift::bench
