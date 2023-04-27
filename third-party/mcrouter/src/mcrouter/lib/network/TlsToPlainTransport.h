/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/AsyncSocket.h>
#include <mcrouter/lib/network/McSSLUtil.h>
#include <mcrouter/lib/network/SecurityOptions.h>

namespace facebook {
namespace memcache {

class TlsToPlainTransport : public folly::AsyncSocket {
 public:
  using UniquePtr = std::
      unique_ptr<TlsToPlainTransport, folly::DelayedDestruction::Destructor>;
  using AsyncSocket::AsyncSocket;

  std::string getSecurityProtocol() const override {
    return McSSLUtil::kTlsToPlainProtocolName;
  }

  void setStats(SecurityTransportStats stats) {
    stats_ = stats;
  }

  SecurityTransportStats getStats() const {
    return stats_;
  }

  void setAddresses(folly::SocketAddress local, folly::SocketAddress peer) {
    localAddr_ = std::move(local);
    addr_ = std::move(peer);
  }

 private:
  SecurityTransportStats stats_;
};

} // namespace memcache
} // namespace facebook
