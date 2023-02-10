/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/transport/AsyncUDPSocketFactory.h"

namespace proxygen {

AsyncUDPSocketFactory::AsyncUDPSocketFactory(
    folly::EventBase* eventBase,
    folly::SocketAddress v6Address,
    folly::Optional<folly::SocketAddress> v4Address)
    : v6Address_(std::move(v6Address)), v4Address_(std::move(v4Address)) {
  CHECK(eventBase);
  eventBase_ = eventBase;
}

folly::Expected<std::unique_ptr<folly::AsyncUDPSocket>, proxygen::Exception>
AsyncUDPSocketFactory::createSocket(
    const folly::SocketAddress& destinationAddress,
    SocketCreateOptions options) {
  std::unique_ptr<folly::AsyncUDPSocket> socket =
      std::make_unique<folly::AsyncUDPSocket>(eventBase_);

  auto maybeAddress = getBindingAddress(destinationAddress);

  if (!maybeAddress.hasValue()) {
    return folly::makeUnexpected(Exception("Unable to create socket error=",
                                           maybeAddress.error().what()));
  }

  try {
    socket->bind(maybeAddress.value());
    if (options.connectSocket) {
      socket->connect(destinationAddress);
    }
  } catch (const std::runtime_error& ex) {
    return folly::makeUnexpected(Exception(ex.what()));
  }

  return socket;
}

folly::Expected<folly::SocketAddress, proxygen::Exception>
AsyncUDPSocketFactory::getBindingAddress(
    const folly::SocketAddress& destination) {

  if (destination.getIPAddress().isV4()) {
    if (!v4Address_.hasValue()) {
      return folly::makeUnexpected(Exception(
          "Bind addresses not provided for address family, destination=",
          destination.getAddressStr()));
    }

    return v4Address_.value();
  }

  return v6Address_;
}

} // namespace proxygen
