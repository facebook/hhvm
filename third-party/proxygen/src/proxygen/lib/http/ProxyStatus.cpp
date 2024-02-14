/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/ProxyStatus.h>

#include <glog/logging.h>

namespace {

constexpr folly::StringPiece kProxyParam{"e_proxy"};
constexpr folly::StringPiece kUpstreamIPParam{"e_upip"};
constexpr folly::StringPiece kIsProxyErrorParam{"e_isproxyerr"};
constexpr folly::StringPiece kIsClientErrorTrue{"e_isclienterr"};
constexpr folly::StringPiece kIsServerErrorTrue{"e_isservererr"};
constexpr folly::StringPiece kClientAddrParam{"e_clientaddr"};

} // namespace

namespace proxygen {

ProxyStatus::ProxyStatus(StatusType statusType) : statusType_{statusType} {
  pIdent_.identifier = getStatusTypeString(statusType_);
}

StatusType ProxyStatus::getStatusType() const {
  return statusType_;
}

void ProxyStatus::setStatusType(StatusType statusType) {
  statusType_ = statusType;
  pIdent_.identifier = getStatusTypeString(statusType_);
}

ProxyStatus& ProxyStatus::setProxyStatusParameter(folly::StringPiece name,
                                                  const std::string& text) {
  if (text.empty()) {
    return *this;
  }

  pIdent_.parameterMap[name.str()] =
      StructuredHeaderItem(StructuredHeaderItem::Type::STRING, text);
  return *this;
}

ProxyStatus& ProxyStatus::setProxy(const std::string& proxy) {
  return setProxyStatusParameter(kProxyParam, proxy);
}

ProxyStatus& ProxyStatus::setUpstreamIP(const std::string& upstreamIP) {
  return setProxyStatusParameter(kUpstreamIPParam, upstreamIP);
}

ProxyStatus& ProxyStatus::setProxyError(const bool isProxyError) {
  if (isProxyError) {
    return setProxyStatusParameter(kIsProxyErrorParam, "true");
  } else {
    return setProxyStatusParameter(kIsProxyErrorParam, "false");
  }
}

ProxyStatus& ProxyStatus::setServerError(const bool isServerError) {
  if (isServerError) {
    return setProxyStatusParameter(kIsServerErrorTrue, "true");
  } else {
    return setProxyStatusParameter(kIsServerErrorTrue, "false");
  }
}

ProxyStatus& ProxyStatus::setClientError(const bool isClientError) {
  if (isClientError) {
    return setProxyStatusParameter(kIsClientErrorTrue, "true");
  } else {
    return setProxyStatusParameter(kIsClientErrorTrue, "false");
  }
}

ProxyStatus& ProxyStatus::setClientAddress(const std::string& clientAddr) {
  return setProxyStatusParameter(kClientAddrParam, clientAddr);
}

bool ProxyStatus::hasUpstreamIP() const {
  return pIdent_.parameterMap.find(std::string(kUpstreamIPParam)) !=
         pIdent_.parameterMap.end();
}

std::string ProxyStatus::toString() const {
  StructuredHeaders::ParameterisedList plist;
  StructuredHeadersEncoder encoder;
  plist.emplace_back(std::move(pIdent_));
  encoder.encodeParameterisedList(plist);

  return encoder.get();
}

bool ProxyStatus::isEmpty() const {
  return statusType_ == StatusType::ENUM_COUNT;
}

} // namespace proxygen
