/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProxyConfigBuilder.h"

#include <folly/json.h>

#include "mcrouter/ConfigApi.h"
#include "mcrouter/PoolFactory.h"
#include "mcrouter/Proxy.h"
#include "mcrouter/ProxyConfig.h"
#include "mcrouter/TargetHooks.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/config/ConfigPreprocessor.h"
#include "mcrouter/lib/fbi/cpp/globals.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/routes/McImportResolver.h"

namespace facebook {
namespace memcache {
namespace mcrouter {
namespace {
const std::string kMcrouterOptionPrefix = "mcr_opts-";

template <class T>
bool tryToString(const boost::any& value, std::string& res) {
  if (boost::any_cast<T*>(&value) != nullptr) {
    res = folly::to<std::string>(*boost::any_cast<T*>(value));
    return true;
  }
  return false;
}
} // namespace

ProxyConfigBuilder::ProxyConfigBuilder(
    const McrouterOptions& opts,
    ConfigApi& configApi,
    folly::StringPiece jsonC,
    const std::string& routerInfoName)
    : json_(nullptr) {
  McImportResolver importResolver(configApi);
  folly::json::metadata_map configMetadataMap;

  auto globalParams = buildGlobalParams(opts, routerInfoName);
  json_ = ConfigPreprocessor::getConfigWithoutMacros(
      jsonC, importResolver, std::move(globalParams), &configMetadataMap);

  poolFactory_ = std::make_unique<PoolFactory>(
      json_, configApi, std::move(configMetadataMap));

  configMd5Digest_ = Md5Hash(jsonC);

  // We rely on consistent naming for named_handles overrides when we build
  // these routes later. See the check in RouteHandleFactory::addNamed.
  if (const auto jNamedHandles = json_.get_ptr("named_handles")) {
    if (jNamedHandles->isObject()) {
      for (auto& it : jNamedHandles->items()) {
        if (it.second.isObject()) {
          const auto jName = it.second.get_ptr("name");
          if (!jName) {
            it.second["name"] = it.first.stringPiece();
          }
        }
      }
    }
  }
}

folly::F14NodeMap<std::string, folly::dynamic>
ProxyConfigBuilder::buildGlobalParams(
    const McrouterOptions& opts,
    const std::string& routerInfoName) {
  int sr_linked = 0;
  if (opts.enable_service_router && mcrouter::gSRInitHook) {
    sr_linked = 1;
  }
  folly::F14NodeMap<std::string, folly::dynamic> globalParams{
      {"default-route", opts.default_route.str()},
      {"default-region", opts.default_route.getRegion().str()},
      {"default-cluster", opts.default_route.getCluster().str()},
      {"hostid", globals::hostid()},
      {"router-name", opts.router_name},
      {"service-name", opts.service_name},
      {"service-router-capable", sr_linked},
      {"router-info-name", routerInfoName}};

  auto additionalParams = additionalConfigParams();
  for (auto& it : additionalParams) {
    globalParams.emplace(it.first, std::move(it.second));
  }

  for (const auto& param : opts.config_params) {
    globalParams.emplace(param.first, param.second);
  }

  opts.forEach([&globalParams](
                   const std::string& name,
                   McrouterOptionData::Type type,
                   const boost::any& value) {
    if (type == McrouterOptionData::Type::integer ||
        type == McrouterOptionData::Type::toggle) {
      std::string res;
      if (tryToString<int64_t>(value, res) || tryToString<bool>(value, res) ||
          tryToString<int>(value, res) || tryToString<uint32_t>(value, res) ||
          tryToString<size_t>(value, res) ||
          tryToString<uint16_t>(value, res) ||
          tryToString<unsigned int>(value, res)) {
        globalParams.emplace(kMcrouterOptionPrefix + name, res);
        return;
      }
      throwLogic(
          "ProxyConfigBuilder::buildGlobalParams: Unsupported option type: {}, {}",
          value.type().name(),
          name);
    }
  });
  return globalParams;
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
