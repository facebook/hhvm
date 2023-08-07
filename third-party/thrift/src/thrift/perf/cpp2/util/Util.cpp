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

#include <thrift/perf/cpp2/util/Util.h>

#include <folly/io/async/fdsock/AsyncFdSocket.h>

namespace apache {
namespace thrift {
namespace perf {

folly::AsyncSocket::UniquePtr getSocket(
    folly::EventBase* evb,
    const folly::SocketAddress& addr,
    bool encrypted,
    std::list<std::string> advertizedProtocols) {
  if (addr.getFamily() == AF_UNIX) {
    // NB: It's technically possibly to use TLS handshake over AF_UNIX to
    // get crypto authentication, but it's not useful in a perf benchmark.
    // https://github.com/facebook/fbthrift/search?q=D46364470&type=commits
    CHECK(!encrypted) << "Not implemented: TLS encryption with AF_UNIX";
    return folly::AsyncSocket::UniquePtr{new folly::AsyncFdSocket(evb, addr)};
  }
  folly::AsyncSocket::UniquePtr sock(new folly::AsyncSocket(evb, addr));
  if (encrypted) {
    auto sslContext = std::make_shared<folly::SSLContext>();
    sslContext->setAdvertisedNextProtocols(advertizedProtocols);
    auto sslSock = new TAsyncSSLSocket(
        sslContext, evb, sock->detachNetworkSocket(), false);
    sslSock->sslConn(nullptr);
    sock.reset(sslSock);
  }
  sock->setZeroCopy(true);
  return sock;
}

} // namespace perf
} // namespace thrift
} // namespace apache
