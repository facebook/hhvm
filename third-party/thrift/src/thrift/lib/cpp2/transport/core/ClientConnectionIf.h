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

#include <memory>

#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/ClientChannel.h>
#include <thrift/lib/cpp2/transport/core/ThriftChannelIf.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

class ThriftClient;

/**
 * The API to the transport layer.
 *
 * Concrete implementations support specific transports such as Proxygen
 * and RSocket.
 *
 * The primary method in this API is "getChannel()" which returns a
 * "ThriftChannelIf" object which is used to interface between the
 * Thrift layer and this layer.  A separate call to "getChannel()"
 * must be made for each RPC.
 *
 * "ClientConnectionIf" objects operate on an event base that must be
 * supplied when an object is constructed - see comments on each
 * subclass for more details.  The event base loop must be running.
 * This is the event base on which callbacks from the networking layer
 * take place.  This is also the event base on which calls to
 * "ThriftChannelIf" objects must be made from ThriftClient objects.
 *
 * Multiple "ClientConnectionIf" objects may be present at the same
 * time, each will manage a separate network connection and are
 * generally on different event bases (threads).
 *
 * All methods must be called on the same event base that manages the
 * connection.
 */
class ClientConnectionIf {
 protected:
  ClientConnectionIf() = default;

 public:
  virtual ~ClientConnectionIf() = default;

  ClientConnectionIf(const ClientConnectionIf&) = delete;
  ClientConnectionIf& operator=(const ClientConnectionIf&) = delete;
  ClientConnectionIf(ClientConnectionIf&&) = delete;
  ClientConnectionIf& operator=(ClientConnectionIf&&) = delete;

  // Returns a channel object for use on a single RPC.  Throws
  // TTransportException if a channel object cannot be returned.
  virtual std::shared_ptr<ThriftChannelIf> getChannel() = 0;

  // Sets the maximum pending outgoing requests allowed on this
  // connection.  Subject to negotiation with the server, which may
  // dictate a smaller maximum.
  virtual void setMaxPendingRequests(uint32_t num) = 0;

  // Sets/updates the close callback on behalf of the client.  Can be
  // called with "cb" set to nullptr to remove the close callback.
  virtual void setCloseCallback(ThriftClient* client, CloseCallback* cb) = 0;

  // Returns the event base of the underlying transport.  This is also
  // the event base on which all calls to channel object (obtained via
  // "getChannel()") must be scheduled.
  virtual folly::EventBase* getEventBase() const = 0;

  // The following methods are proxies for ClientChannel methods.
  // They are called from corresponding "ThriftClient" methods, which
  // in turn can be used from any client side connection management
  // framework.
  virtual folly::AsyncTransport* getTransport() = 0;
  virtual bool good() = 0;
  virtual ClientChannel::SaturationStatus getSaturationStatus() = 0;
  virtual void attachEventBase(folly::EventBase* evb) = 0;
  virtual void detachEventBase() = 0;
  virtual bool isDetachable() = 0;
  virtual void closeNow() = 0;
  virtual CLIENT_TYPE getClientType() = 0;

  // Client timeouts for read, write.
  virtual uint32_t getTimeout() = 0;
  virtual void setTimeout(uint32_t ms) = 0;
};

} // namespace apache::thrift
