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

#pragma once

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/net/NetworkSocket.h>
#include <thrift/lib/cpp/transport/TTransportException.h>

namespace apache {
namespace thrift {
namespace async {

// Wrapper around folly's AsyncSSLSocket to maintain backwards compatibility:
// Converts exceptions to thrift's TTransportException type.
class TAsyncSSLSocket : public folly::AsyncSSLSocket {
 public:
  typedef std::unique_ptr<TAsyncSSLSocket, Destructor> UniquePtr;

  explicit TAsyncSSLSocket(
      const std::shared_ptr<folly::SSLContext>& ctx, folly::EventBase* evb)
      : folly::AsyncSSLSocket(ctx, evb) {}

  TAsyncSSLSocket(
      const std::shared_ptr<folly::SSLContext>& ctx,
      AsyncSocket* oldAsyncSocket)
      : folly::AsyncSSLSocket(ctx, oldAsyncSocket) {}

  TAsyncSSLSocket(
      const std::shared_ptr<folly::SSLContext>& ctx,
      folly::EventBase* evb,
      folly::NetworkSocket fd,
      bool server = true,
      bool deferSecurityNegotiation = false,
      const folly::SocketAddress* peerAddress = nullptr)
      : folly::AsyncSSLSocket(
            ctx, evb, fd, server, deferSecurityNegotiation, peerAddress) {}

  TAsyncSSLSocket(
      const std::shared_ptr<folly::SSLContext>& ctx,
      folly::EventBase* evb,
      folly::AsyncSSLSocket::Options&& options)
      : folly::AsyncSSLSocket(ctx, evb, std::move(options)) {}

  static TAsyncSSLSocket::UniquePtr newSocket(
      const std::shared_ptr<folly::SSLContext>& ctx, folly::EventBase* evb) {
    return TAsyncSSLSocket::UniquePtr(new TAsyncSSLSocket(ctx, evb));
  }

  static TAsyncSSLSocket::UniquePtr newSocket(
      const std::shared_ptr<folly::SSLContext>& ctx,
      folly::EventBase* evb,
      folly::NetworkSocket fd,
      bool server = true) {
    return TAsyncSSLSocket::UniquePtr(
        new TAsyncSSLSocket(ctx, evb, fd, server));
  }

#if OPENSSL_VERSION_NUMBER >= 0x1000105fL && !defined(OPENSSL_NO_TLSEXT)
  TAsyncSSLSocket(
      const std::shared_ptr<folly::SSLContext>& ctx,
      folly::EventBase* evb,
      const std::string& serverName)
      : folly::AsyncSSLSocket(ctx, evb, serverName) {}

  TAsyncSSLSocket(
      const std::shared_ptr<folly::SSLContext>& ctx,
      folly::EventBase* evb,
      folly::NetworkSocket fd,
      const std::string& serverName)
      : folly::AsyncSSLSocket(ctx, evb, fd, serverName) {}

  static TAsyncSSLSocket::UniquePtr newSocket(
      const std::shared_ptr<folly::SSLContext>& ctx,
      folly::EventBase* evb,
      const std::string& serverName) {
    return TAsyncSSLSocket::UniquePtr(
        new TAsyncSSLSocket(ctx, evb, serverName));
  }
#endif
};

} // namespace async
} // namespace thrift
} // namespace apache
