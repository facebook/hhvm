/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JsonClient.h"

#include <folly/fibers/FiberManagerMap.h>

#include "mcrouter/lib/network/AsyncMcClient.h"
#include "mcrouter/lib/network/ConnectionOptions.h"

using facebook::memcache::ConnectionOptions;
using facebook::memcache::SecurityMech;

namespace carbon {

namespace {
ConnectionOptions getConnectionOptions(const JsonClient::Options& opts) {
  auto mech = opts.useSsl ? SecurityMech::TLS : SecurityMech::NONE;
  ConnectionOptions options(opts.host, opts.port, mc_caret_protocol, mech);
  if (opts.useSsl) {
    options.securityOpts.sslPemCertPath = opts.pemCertPath;
    options.securityOpts.sslPemKeyPath = opts.pemKeyPath;
    options.securityOpts.sslPemCaPath = opts.pemCaPath;
    options.securityOpts.sslServiceIdentity = opts.sslServiceIdentity;
    options.securityOpts.sessionCachingEnabled = true;
    options.securityOpts.tfoEnabledForSsl = true;
  }
  return options;
}
} // anonymous namespace

JsonClient::JsonClient(
    JsonClient::Options options,
    std::function<void(const std::string& msg)> onError)
    : options_{std::move(options)},
      onError_{std::move(onError)},
      evb_{/* enableTimeMeasurement */ false},
      client_{evb_, getConnectionOptions(options_)},
      fiberManager_{folly::fibers::getFiberManager(evb_)} {}

bool JsonClient::sendRequests(
    const std::string& requestName,
    const folly::dynamic& requests,
    folly::dynamic& replies) {
  if (!requests.isArray()) {
    return sendRequestByName(requestName, requests, replies);
  }

  replies = folly::dynamic::array();
  for (size_t i = 0; i < requests.size(); ++i) {
    folly::dynamic reply;
    if (!sendRequestByName(requestName, requests[i], reply)) {
      return false;
    }
    replies.push_back(std::move(reply));
  }
  return true;
}

void JsonClient::onError(const std::string& msg) const {
  if (onError_) {
    onError_(msg);
  }
}

} // namespace carbon
