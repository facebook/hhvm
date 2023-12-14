/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AccessPoint.h"

#include <folly/Conv.h>
#include <folly/IPAddress.h>
#include <folly/IPAddressException.h>

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {

namespace {

void parseParts(folly::StringPiece s) {
  if (!s.empty()) {
    throw std::runtime_error("Invalid AccessPoint format");
  }
}

template <class... Args>
void parseParts(folly::StringPiece s, folly::StringPiece& out, Args&... args) {
  if (s.empty()) {
    return;
  }
  if (s[0] != ':') {
    throw std::runtime_error("Invalid AccessPoint format");
  }
  s.advance(1);
  auto colon = s.find(":");
  if (colon == std::string::npos) {
    out = s;
  } else {
    out = s.subpiece(0, colon);
    s.advance(colon);
    parseParts(s, args...);
  }
}

bool parseCompressed(folly::StringPiece s) {
  if (s == "compressed") {
    return true;
  } else if (s == "notcompressed") {
    return false;
  }
  throw std::runtime_error("Invalid compression config");
}

mc_protocol_t parseProtocol(folly::StringPiece str) {
  if (str == "ascii") {
    return mc_ascii_protocol;
  } else if (str == "caret") {
    return mc_caret_protocol;
  } else if (str == "thrift") {
    return mc_thrift_protocol;
  }
  throw std::runtime_error("Invalid protocol");
}

} // namespace

AccessPoint::AccessPoint(
    folly::StringPiece host,
    uint16_t port,
    mc_protocol_t protocol,
    SecurityMech mech,
    bool compressed,
    bool unixDomainSocket,
    uint32_t failureDomain,
    std::optional<uint16_t> taskId,
    std::optional<std::string> serviceIdOverride)
    : port_(port),
      protocol_(protocol),
      securityMech_(mech),
      compressed_(compressed),
      unixDomainSocket_(unixDomainSocket),
      failureDomain_(failureDomain),
      taskId_(taskId),
      serviceId_(serviceIdOverride) {
  auto const maybe_ip = folly::IPAddress::tryFromString(host);
  if (maybe_ip.hasError()) {
    // host is not an IP address (e.g. 'localhost')
    host_ = host.str();
    isV6_ = false;
  } else {
    auto const& ip = maybe_ip.value();
    host_ = ip.toFullyQualified();
    hash_ = folly::hash_value(ip);
    isV6_ = ip.isV6();
  }
}

AccessPoint::AccessPoint(
    const folly::IPAddress& ip,
    uint16_t port,
    uint32_t failureDomain,
    mc_protocol_t protocol,
    std::optional<uint16_t> taskId,
    std::optional<std::string> serviceIdOverride)
    : port_(port),
      protocol_(protocol),
      failureDomain_(failureDomain),
      taskId_(taskId),
      serviceId_(serviceIdOverride) {
  host_ = ip.toFullyQualified();
  hash_ = folly::hash_value(ip);
  isV6_ = ip.isV6();
}

std::shared_ptr<AccessPoint> AccessPoint::create(
    folly::StringPiece apString,
    mc_protocol_t defaultProtocol,
    SecurityMech defaultMech,
    uint16_t portOverride,
    bool defaultCompressed,
    uint32_t failureDomain,
    std::optional<uint16_t> taskId,
    std::optional<std::string> serviceIdOverride) {
  if (apString.empty()) {
    return nullptr;
  }

  folly::StringPiece host;
  bool unixDomainSocket = false;
  if (apString[0] == '[') {
    // IPv6
    auto closing = apString.find(']');
    if (closing == std::string::npos) {
      return nullptr;
    }
    host = apString.subpiece(1, closing - 1);
    apString.advance(closing + 1);
  } else {
    // IPv4 or hostname or UNIX domain socket
    if (apString.subpiece(0, 5) == "unix:") { // Unix domain socket
      unixDomainSocket = true;
      apString.advance(5);
    }
    auto colon = apString.find(':');
    if (colon == std::string::npos) {
      host = apString;
      apString = "";
    } else {
      host = apString.subpiece(0, colon);
      apString.advance(colon);
    }
  }

  if (host.empty()) {
    return nullptr;
  }

  try {
    folly::StringPiece port, protocol, encr, comp;
    if (unixDomainSocket) {
      port = "0";
      parseParts(apString, protocol, encr, comp);
      // Unix Domain Sockets with SSL is not supported.
      if (!encr.empty() && parseSecurityMech(encr) != SecurityMech::NONE) {
        return nullptr;
      }
    } else {
      parseParts(apString, port, protocol, encr, comp);
    }

    return std::make_shared<AccessPoint>(
        host,
        portOverride != 0 ? portOverride : folly::to<uint16_t>(port),
        protocol.empty() ? defaultProtocol : parseProtocol(protocol),
        encr.empty() ? defaultMech : parseSecurityMech(encr),
        comp.empty() ? defaultCompressed : parseCompressed(comp),
        unixDomainSocket,
        failureDomain,
        taskId,
        serviceIdOverride);
  } catch (const std::exception&) {
    return nullptr;
  }
}

void AccessPoint::disableCompression() {
  compressed_ = false;
}

std::string AccessPoint::toHostPortString() const {
  if (isV6_) {
    return folly::to<std::string>("[", host_, "]:", port_);
  }
  return folly::to<std::string>(host_, ":", port_);
}

std::string AccessPoint::toString() const {
  assert(protocol_ != mc_unknown_protocol);
  if (isV6_) {
    return folly::to<std::string>(
        "[",
        host_,
        "]:",
        port_,
        ":",
        mc_protocol_to_string(protocol_),
        ":",
        securityMechToString(securityMech_),
        ":",
        compressed_ ? "compressed" : "notcompressed");
  }
  return folly::to<std::string>(
      host_,
      ":",
      port_,
      ":",
      mc_protocol_to_string(protocol_),
      ":",
      securityMechToString(securityMech_),
      ":",
      compressed_ ? "compressed" : "notcompressed");
}

const AccessPoint& AccessPoint::defaultAp() {
  static const AccessPoint ap =
      AccessPoint(folly::IPAddress(), 0, 0, mc_thrift_protocol);
  return ap;
}

} // namespace memcache
} // namespace facebook
