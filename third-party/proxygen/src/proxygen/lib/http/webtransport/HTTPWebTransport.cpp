/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/webtransport/HTTPWebTransport.h>

#include <proxygen/lib/http/HTTPMessage.h>

namespace proxygen {

/*static*/ bool HTTPWebTransport::isConnectMessage(const HTTPMessage& msg) {
  constexpr std::string_view kWebTransport{"webtransport"};
  return msg.isRequest() && msg.getMethod() == proxygen::HTTPMethod::CONNECT &&
         msg.getUpgradeProtocol() && *msg.getUpgradeProtocol() == kWebTransport;
}

}; // namespace proxygen
