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

#include <stdint.h>

#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

class ThriftClientCallback;

/**
 * Interface used by the Thrift layer to communicate with the
 * Transport layer.  The same interface is used both on the client
 * side and the server side.
 *
 * On the client side, this interface is used to send a request to the
 * server, while on the server side it is used to send a response to
 * the client.
 *
 * Implementations are specialized for different kinds of transports
 * (e.g., HTTP/2, RSocket) and using different strategies (e.g., one
 * RPC per HTTP/2 stream, or multiple RPCs per HTTP/2 stream).  The
 * same implementation is used on both the client and the server side.
 *
 * The lifetime of a channel object depends on the implementation - on
 * the one extreme, a new object may be created for each RPC, while on
 * the other extreme a single object may be use for the lifetime of a
 * connection.
 */
class ThriftChannelIf : public std::enable_shared_from_this<ThriftChannelIf> {
 public:
  struct RequestMetadata {
    RequestRpcMetadata requestRpcMetadata;
    std::string host;
    std::string url;
  };

  ThriftChannelIf() {}

  virtual ~ThriftChannelIf() = default;

  ThriftChannelIf(const ThriftChannelIf&) = delete;
  ThriftChannelIf& operator=(const ThriftChannelIf&) = delete;

  // Called on the server at the end of a single response RPC.  This
  // is not called for streaming response and no response RPCs.
  //
  // Calls must be scheduled on the event base obtained from
  // "getEventBase()".
  virtual void sendThriftResponse(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> payload) noexcept = 0;

  // Called from the client to initiate an RPC with a server.
  // "callback" is used to call back with the response for single
  // response RPCs.  "callback" is not used for streaming response and
  // no response RPCs.
  //
  // Calls to the channel must be scheduled "this->getEventBase()".
  // Callbacks to "callback" must be scheduled on
  // "callback->getEventBase()".
  //
  // "callback" must not be destroyed until it has received the call
  // back to "onThriftResponse()".
  virtual void sendThriftRequest(
      RequestMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> payload,
      std::unique_ptr<ThriftClientCallback> callback) noexcept = 0;

  // Returns the event base on which calls to "sendThriftRequest()"
  // and "sendThriftResponse()" must be scheduled.
  virtual folly::EventBase* getEventBase() noexcept = 0;
};

} // namespace apache::thrift
