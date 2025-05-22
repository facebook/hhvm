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

#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/SSLContext.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache::thrift::python::client {

class ChannelFactory {
 public:
  virtual ~ChannelFactory() {}
  /**
   * Create a thrift channel by connecting to a host:port over TCP.
   */
  virtual folly::Future<apache::thrift::RequestChannel::Ptr>
  createThriftChannelTCP(
      const std::string& host,
      uint16_t port,
      uint32_t connect_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto,
      const std::string& endpoint) = 0;

  apache::thrift::RequestChannel::Ptr sync_createThriftChannelTCP(
      const std::string& host,
      uint16_t port,
      uint32_t connect_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto,
      const std::string& endpoint);

  /**
   * Create a thrift channel by connecting to a Unix domain socket.
   */
  virtual folly::Future<apache::thrift::RequestChannel::Ptr>
  createThriftChannelUnix(
      const std::string& path,
      uint32_t connect_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto) = 0;

  apache::thrift::RequestChannel::Ptr sync_createThriftChannelUnix(
      const std::string& path,
      uint32_t connect_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto);

  /**
   * Create a thrift channel by connecting to a host:port over TCP then SSL.
   */
  virtual folly::Future<apache::thrift::RequestChannel::Ptr>
  createThriftChannelSSL(
      const std::shared_ptr<folly::SSLContext>& ctx,
      const std::string& host,
      const uint16_t port,
      const uint32_t connect_timeout,
      const uint32_t ssl_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto,
      const std::string& endpoint) = 0;

  apache::thrift::RequestChannel::Ptr sync_createThriftChannelSSL(
      const std::shared_ptr<folly::SSLContext>& ctx,
      const std::string& host,
      const uint16_t port,
      const uint32_t connect_timeout,
      const uint32_t ssl_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto,
      const std::string& endpoint);
};

class DefaultChannelFactory : public ChannelFactory {
 public:
  folly::Future<apache::thrift::RequestChannel::Ptr> createThriftChannelTCP(
      const std::string& host,
      uint16_t port,
      uint32_t connect_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto,
      const std::string& endpoint) override;

  folly::Future<apache::thrift::RequestChannel::Ptr> createThriftChannelUnix(
      const std::string& path,
      uint32_t connect_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto) override;

  folly::Future<apache::thrift::RequestChannel::Ptr> createThriftChannelSSL(
      const std::shared_ptr<folly::SSLContext>& ctx,
      const std::string& host,
      const uint16_t port,
      const uint32_t connect_timeout,
      const uint32_t ssl_timeout,
      CLIENT_TYPE client_t,
      apache::thrift::protocol::PROTOCOL_TYPES proto,
      const std::string& endpoint) override;

  ~DefaultChannelFactory() override = default;
};

} // namespace apache::thrift::python::client
