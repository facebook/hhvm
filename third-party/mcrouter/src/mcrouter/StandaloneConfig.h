/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <folly/Range.h>
#include <folly/io/async/AsyncTransport.h>

namespace apache {
namespace thrift {
class ThriftServer;
class Cpp2RequestContext;
} // namespace thrift
} // namespace apache

namespace facebook {
namespace memcache {

// forward declarations
class McrouterOptions;
class McServerSession;

namespace mcrouter {
// forward declarations
class McrouterStandaloneOptions;
class ExternalStatsHandler;

void standalonePreInitFromCommandLineOpts(
    const std::unordered_map<std::string, std::string>& standaloneOptionsDict);

void standaloneInit(
    const McrouterOptions& opts,
    const McrouterStandaloneOptions& standaloneOpts);

void initStandaloneSSL();

void initStandaloneSSLDualServer(
    const McrouterStandaloneOptions& standaloneOpts,
    std::shared_ptr<apache::thrift::ThriftServer> thriftServer);

void finalizeStandaloneOptions(McrouterStandaloneOptions& opts);

std::function<void(McServerSession&)> getConnectionAclChecker(
    const std::string& serviceIdentity,
    bool enforce);

std::function<bool(const folly::AsyncTransportWrapper*)>
getThriftConnectionAclChecker(const std::string& serviceIdentity, bool enforce);

using MemcacheRequestAclCheckerCallback = std::function<
    bool(const apache::thrift::Cpp2RequestContext*, const folly::StringPiece)>;
MemcacheRequestAclCheckerCallback getMemcacheServerRequestAclCheckCallback(
    ExternalStatsHandler& statsHandler);

void refreshMemcacheServerRequestAclChecker();

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
