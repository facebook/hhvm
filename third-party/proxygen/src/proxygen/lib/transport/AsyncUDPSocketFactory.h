/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Expected.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncUDPSocket.h>
#include <proxygen/lib/utils/Exception.h>

namespace proxygen {

class AsyncUDPSocketFactory {
 public:
  struct SocketCreateOptions {
    bool connectSocket{false};
  };

  explicit AsyncUDPSocketFactory(
      folly::EventBase* eventBase,
      folly::SocketAddress v6BindAddress,
      folly::Optional<folly::SocketAddress> v4BindAddress = folly::none);
  ~AsyncUDPSocketFactory() = default;

  folly::Expected<std::unique_ptr<folly::AsyncUDPSocket>, proxygen::Exception>
  createSocket(const folly::SocketAddress& destinationAddress,
               SocketCreateOptions options = getDefaultCreateOptions());

 private:
  static SocketCreateOptions getDefaultCreateOptions() {
    return SocketCreateOptions{.connectSocket = false};
  }

  folly::Expected<folly::SocketAddress, proxygen::Exception> getBindingAddress(
      const folly::SocketAddress& destination);

  folly::EventBase* eventBase_{nullptr};
  folly::SocketAddress v6Address_;
  folly::Optional<folly::SocketAddress> v4Address_;
};
} // namespace proxygen
