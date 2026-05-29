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

#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>

#include <memory>

namespace apache::thrift::fast_thrift::bench {

/**
 * Base class for benchmark clients.
 *
 * This provides the common interface used by BenchmarkSuite.
 * Specific implementations (TcpClient, RocketClient, etc.) inherit from this.
 */
class Client {
 public:
  virtual ~Client() = default;

  Client() = default;
  Client(Client&&) = default;
  Client& operator=(Client&&) = default;
  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;

  /**
   * Send a payload and receive the echoed response.
   * Must be called from the EventBase thread.
   */
  virtual folly::Future<std::unique_ptr<folly::IOBuf>> echo(
      std::unique_ptr<folly::IOBuf> data) = 0;

  /**
   * Shutdown the connection and release resources.
   * Must be called from the EventBase thread.
   */
  virtual void shutdown() = 0;
};

} // namespace apache::thrift::fast_thrift::bench
