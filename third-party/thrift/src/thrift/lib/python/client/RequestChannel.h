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
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace thrift {
namespace python {
namespace client {

/**
 * Create a thrift channel by connecting to a host:port over TCP.
 */
folly::Future<apache::thrift::RequestChannel::Ptr> createThriftChannelTCP(
    const std::string& host,
    uint16_t port,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    const std::string& endpoint);

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
folly::Future<apache::thrift::RequestChannel::Ptr> createThriftChannelUnix(
    const std::string& path,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto);

apache::thrift::RequestChannel::Ptr sync_createThriftChannelUnix(
    const std::string& path,
    uint32_t connect_timeout,
    CLIENT_TYPE client_t,
    apache::thrift::protocol::PROTOCOL_TYPES proto);

apache::thrift::RequestChannel::Ptr createHeaderChannel(
    folly::AsyncTransport::UniquePtr sock,
    CLIENT_TYPE client,
    apache::thrift::protocol::PROTOCOL_TYPES proto,
    folly::Optional<std::string> host = folly::none,
    folly::Optional<std::string> endpoint = folly::none);

} // namespace client
} // namespace python
} // namespace thrift
