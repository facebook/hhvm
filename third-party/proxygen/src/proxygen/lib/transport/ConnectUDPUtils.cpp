/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/ConnectUDPUtils.h>

#include <folly/String.h>
#include <folly/io/Cursor.h>
#include <quic/codec/QuicInteger.h>
#include <quic/folly_utils/Utils.h>

namespace proxygen {

namespace {
// Percent-encode colons in an IPv6 address as %3A
std::string percentEncodeIPv6(const std::string& host) {
  std::string result;
  result.reserve(host.size() * 3);
  for (char c : host) {
    if (c == ':') {
      result.append("%3A");
    } else {
      result.push_back(c);
    }
  }
  return result;
}
} // namespace

ConnectUDPTarget expandConnectUDPTemplate(const std::string& uriTemplate,
                                          const std::string& targetHost,
                                          uint16_t targetPort) {
  // Encode IPv6 colons
  std::string encodedHost = (targetHost.find(':') != std::string::npos)
                                ? percentEncodeIPv6(targetHost)
                                : targetHost;
  auto portStr = folly::to<std::string>(targetPort);

  // Perform template variable substitution
  std::string expanded = uriTemplate;
  {
    auto pos = expanded.find("{target_host}");
    while (pos != std::string::npos) {
      expanded.replace(pos, 13, encodedHost);
      pos = expanded.find("{target_host}", pos + encodedHost.size());
    }
  }
  {
    auto pos = expanded.find("{target_port}");
    while (pos != std::string::npos) {
      expanded.replace(pos, 13, portStr);
      pos = expanded.find("{target_port}", pos + portStr.size());
    }
  }

  ConnectUDPTarget result;

  // Parse scheme
  auto schemeEnd = expanded.find("://");
  if (schemeEnd == std::string::npos) {
    // No scheme found, return expanded as path
    result.path = expanded;
    return result;
  }
  result.scheme = expanded.substr(0, schemeEnd);

  // Parse authority (host:port)
  auto authorityStart = schemeEnd + 3;
  auto pathStart = expanded.find('/', authorityStart);
  if (pathStart == std::string::npos) {
    result.authority = expanded.substr(authorityStart);
    result.path = "/";
  } else {
    result.authority =
        expanded.substr(authorityStart, pathStart - authorityStart);
    result.path = expanded.substr(pathStart);
  }

  return result;
}

std::unique_ptr<folly::IOBuf> prependContextId(
    std::unique_ptr<folly::IOBuf> payload) {
  auto contextIdBuf = folly::IOBuf::create(1);
  folly::io::Appender appender(contextIdBuf.get(), 1);
  auto res =
      quic::encodeQuicInteger(0, [&](auto val) { appender.writeBE(val); });
  CHECK(res.has_value());
  contextIdBuf->appendToChain(std::move(payload));
  return contextIdBuf;
}

folly::Optional<std::unique_ptr<folly::IOBuf>> stripContextId(
    std::unique_ptr<folly::IOBuf> datagram) {
  if (!datagram || datagram->computeChainDataLength() == 0) {
    return folly::none;
  }

  folly::io::Cursor cursor(datagram.get());
  auto contextIdResult = quic::follyutils::decodeQuicInteger(cursor);
  if (!contextIdResult) {
    return folly::none;
  }

  auto [contextId, varintLen] = *contextIdResult;
  if (contextId != 0) {
    return folly::none;
  }

  // Return the remaining payload after the context ID
  folly::IOBufQueue queue;
  queue.append(std::move(datagram));
  queue.trimStart(varintLen);
  auto result = queue.move();
  if (!result) {
    result = folly::IOBuf::create(0);
  }
  return result;
}

} // namespace proxygen
