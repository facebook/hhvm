/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef THRIFT_TRANSPORT_TRPCTRANSPORT_H_
#define THRIFT_TRANSPORT_TRPCTRANSPORT_H_ 1

#include <thrift/lib/cpp/transport/TTransport.h>

namespace folly {
class SocketAddress;
}

namespace apache {
namespace thrift {
namespace transport {

/**
 * A TRpcTransport adds a getPeerAddress() method to the base TTransport
 * interface.
 */
class TRpcTransport : public TTransport {
 public:
  /**
   * Get the address of the peer to which this transport is connected.
   *
   * @return Returns a pointer to a folly::SocketAddress.  This struct is owned
   * by the TRpcTransport and is guaranteed to remain valid for the lifetime of
   * the TRpcTransport.  It is guaranteed to return non-nullptr.  (On error, a
   * TTransportException will be raised.)
   */
  virtual const folly::SocketAddress* getPeerAddress() = 0;
};

} // namespace transport
} // namespace thrift
} // namespace apache

#endif // THRIFT_TRANSPORT_TRPCTRANSPORT_H_
