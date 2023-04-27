/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "StandaloneConfig.h"

#include <functional>
#include <unordered_map>

namespace facebook {
namespace memcache {
namespace mcrouter {

void standalonePreInitFromCommandLineOpts(
    const std::unordered_map<std::string, std::string>& standaloneOptionsDict) {
}

void standaloneInit(
    const McrouterOptions& opts,
    const McrouterStandaloneOptions& standaloneOpts) {}

void initStandaloneSSL() {}

void initStandaloneSSLDualServer(
    const McrouterStandaloneOptions& /* standaloneOpts */,
    std::shared_ptr<apache::thrift::ThriftServer> /* thriftServer */) {}

void finalizeStandaloneOptions(McrouterStandaloneOptions& opts) {}

std::function<void(McServerSession&)> getConnectionAclChecker(
    const std::string& /* serviceIdentity */,
    bool /* enforce */) {
  return [](McServerSession&) {};
}
std::function<bool(const folly::AsyncTransportWrapper*)>
getThriftConnectionAclChecker(
    const std::string& /* serviceIdentity */,
    bool /* enforce */) {
  return [](const folly::AsyncTransportWrapper*) { return true; };
}

MemcacheRequestAclCheckerCallback getMemcacheServerRequestAclCheckCallback(
    ExternalStatsHandler&) {
  return {};
}

void refreshMemcacheServerRequestAclChecker() {}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
