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
#include <vector>

#include <folly/Format.h>
#include <folly/Range.h>
#include <folly/json/json.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/HostWithShard-fwd.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/Proxy.h"
#include "mcrouter/ProxyConfigBuilder.h"
#include "mcrouter/ProxyRequestContextTyped.h"
#include "mcrouter/config-impl.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/CarbonMessageConversionUtils.h"
#include "mcrouter/lib/fbi/cpp/globals.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/TypedMsg.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/ProxyRoute.h"
#include "mcrouter/standalone_options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {
namespace detail {

bool srHostWithShardFuncRouteHandlesCommandDispatcher(
    const HostWithShard& hostWithShard,
    std::string& tree,
    const int level);

template <class RouterInfo>
class RouteHandlesCommandDispatcher {
 public:
  bool dispatch(
      size_t typeId,
      folly::StringPiece key,
      const ProxyRoute<RouterInfo>& proxyRoute,
      std::string& outStr) {
    return dispatcher_.dispatch(typeId, *this, key, proxyRoute, outStr);
  }

  template <class Request>
  static void processMsg(
      RouteHandlesCommandDispatcher<RouterInfo>& me,
      folly::StringPiece key,
      const ProxyRoute<RouterInfo>& proxyRoute,
      std::string& outStr) {
    outStr = me.processMsgInternal<Request>(key, proxyRoute);
  }

 private:
  CallDispatcher<
      // Request List
      typename RouterInfo::RoutableRequests,
      // Dispatcher class
      detail::RouteHandlesCommandDispatcher<RouterInfo>,
      // List of types of args to Dispatcher::processMsg()
      folly::StringPiece,
      const ProxyRoute<RouterInfo>&,
      std::string&>
      dispatcher_;

  template <class Request>
  std::string processMsgInternal(
      folly::StringPiece key,
      const ProxyRoute<RouterInfo>& proxyRoute) {
    std::string tree;
    int level = 0;
    Request request;
    RouteHandleTraverser<typename RouterInfo::RouteHandleIf> t(
        [&tree, &level](const typename RouterInfo::RouteHandleIf& rh) {
          tree.append(std::string(level, ' ') + rh.routeName() + '\n');
          ++level;
        },
        [&level]() { --level; },
        nullptr,
        [&tree, &level](
            const HostWithShard& hostWithShard,
            const RequestClass& /* unused */) {
          return srHostWithShardFuncRouteHandlesCommandDispatcher(
              hostWithShard, tree, level);
        },
        [&tree, &level](
            const AccessPoint& srHost, const RequestClass& /* unused */) {
          tree.append(
              std::string(level, ' ') +
              "host: " + folly::to<std::string>(srHost.getHost()) +
              " port:" + folly::to<std::string>(srHost.getPort()) + '\n');
          return false;
        });
    if (!key.empty() && key[0] == '{' && key[key.size() - 1] == '}') {
      carbon::convertFromFollyDynamic(folly::parseJson(key), request);
    } else {
      request.key_ref() = key;
    }
    proxyRoute.traverse(request, t);
    return tree;
  }
};

template <class RouterInfo>
class RouteCommandDispatcher {
 public:
  bool dispatch(
      size_t typeId,
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
      folly::StringPiece keyStr,
      Proxy<RouterInfo>& proxy,
      const ProxyRoute<RouterInfo>& proxyRoute) {
    return dispatcher_.dispatch(typeId, *this, ctx, keyStr, proxy, proxyRoute);
  }

  template <class Request>
  static void processMsg(
      RouteCommandDispatcher<RouterInfo>& /* me */,
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
      folly::StringPiece keyStr,
      Proxy<RouterInfo>& proxy,
      const ProxyRoute<RouterInfo>& proxyRoute) {
    proxy.fiberManager().addTaskFinally(
        [keyStr, &proxy, &proxyRoute]() {
          auto destinations = std::make_unique<std::vector<std::string>>();
          folly::fibers::Baton baton;
          auto rctx =
              ProxyRequestContextWithInfo<RouterInfo>::createRecordingNotify(
                  proxy,
                  baton,
                  [&destinations](const PoolContext&, const AccessPoint& dest) {
                    destinations->push_back(dest.toHostPortString());
                  });
          Request recordingReq;
          if (!keyStr.empty() && keyStr[0] == '{' &&
              keyStr[keyStr.size() - 1] == '}') {
            carbon::convertFromFollyDynamic(
                folly::parseJson(keyStr), recordingReq);
          } else {
            recordingReq.key_ref() = keyStr;
          }
          fiber_local<RouterInfo>::runWithLocals(
              [ctx = std::move(rctx), &recordingReq, &proxyRoute]() mutable {
                fiber_local<RouterInfo>::setSharedCtx(std::move(ctx));
                /* ignore the reply */
                proxyRoute.route(recordingReq);
              });
          baton.wait();
          return destinations;
        },
        [ctx](folly::Try<std::unique_ptr<std::vector<std::string>>>&& data) {
          std::string str;
          const auto& destinations = *data;
          for (const auto& d : *destinations) {
            if (!str.empty()) {
              str.push_back('\r');
              str.push_back('\n');
            }
            str.append(d);
          }
          ReplyT<ServiceInfoRequest> reply(carbon::Result::FOUND);
          reply.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, str);
          ctx->sendReply(std::move(reply));
        });
  }

 private:
  CallDispatcher<
      // Request List
      typename RouterInfo::RoutableRequests,
      // Dispatcher class
      detail::RouteCommandDispatcher<RouterInfo>,
      // List of types of args to Dispatcher::processMsg()
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>&,
      folly::StringPiece,
      Proxy<RouterInfo>&,
      const ProxyRoute<RouterInfo>&>
      dispatcher_;
};

template <class RouterInfo>
class GetBucketCommandDispatcher {
 public:
  bool dispatch(
      size_t typeId,
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
      folly::StringPiece keyStr,
      Proxy<RouterInfo>& proxy,
      const ProxyRoute<RouterInfo>& proxyRoute) {
    return dispatcher_.dispatch(typeId, *this, ctx, keyStr, proxy, proxyRoute);
  }

  template <class Request>
  static void processMsg(
      GetBucketCommandDispatcher<RouterInfo>& /* me */,
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
      folly::StringPiece keyStr,
      Proxy<RouterInfo>& proxy,
      const ProxyRoute<RouterInfo>& proxyRoute) {
    using KeyBucketKeyspaceTuple =
        std::tuple<std::string, std::string, std::string>;
    proxy.fiberManager().addTaskFinally(
        [keyStr, &proxy, &proxyRoute]() {
          auto keyBucketKeyspaceTuples =
              std::make_unique<std::vector<KeyBucketKeyspaceTuple>>();
          auto cb = [&keyBucketKeyspaceTuples](
                        std::string keyRecorded,
                        const uint64_t bucketIdRecorded,
                        const std::string_view bucketizationKeyspaceRecorded) {
            keyBucketKeyspaceTuples->push_back(std::make_tuple(
                std::move(keyRecorded),
                folly::to<std::string>(bucketIdRecorded),
                folly::to<std::string>(bucketizationKeyspaceRecorded)));
          };
          folly::fibers::Baton baton;
          auto rctx =
              ProxyRequestContextWithInfo<RouterInfo>::createRecordingNotify(
                  proxy, baton, nullptr, nullptr, std::move(cb));
          Request recordingReq;
          recordingReq.key_ref() = keyStr;
          fiber_local<RouterInfo>::runWithLocals(
              [ctx = std::move(rctx), &recordingReq, &proxyRoute]() mutable {
                fiber_local<RouterInfo>::setSharedCtx(std::move(ctx));
                proxyRoute.route(recordingReq);
              });
          baton.wait();
          return keyBucketKeyspaceTuples;
        },
        [ctx](folly::Try<std::unique_ptr<std::vector<KeyBucketKeyspaceTuple>>>&&
                  data) {
          std::string str;
          const auto& tuplesPtr = *data;
          for (const auto& [key, bucket, keyspace] : *tuplesPtr) {
            if (!str.empty()) {
              str.append("\r\n");
            }
            str.append(key);
            str.push_back('\t');
            str.append(bucket);
            str.push_back('\t');
            str.append(keyspace);
          }
          ReplyT<ServiceInfoRequest> reply(carbon::Result::FOUND);
          reply.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, str);
          ctx->sendReply(std::move(reply));
        });
  }

 private:
  CallDispatcher<
      // Request List
      typename RouterInfo::RoutableRequests,
      // Dispatcher class
      detail::GetBucketCommandDispatcher<RouterInfo>,
      // List of types of args to Dispatcher::processMsg()
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>&,
      folly::StringPiece,
      Proxy<RouterInfo>&,
      const ProxyRoute<RouterInfo>&>
      dispatcher_;
};

} // namespace detail

template <class RouterInfo>
struct ServiceInfo<RouterInfo>::ServiceInfoImpl {
  Proxy<RouterInfo>& proxy_;
  ProxyRoute<RouterInfo>& proxyRoute_;
  std::unordered_map<
      std::string,
      std::function<std::string(const std::vector<folly::StringPiece>& args)>>
      commands_;

  detail::RouteHandlesCommandDispatcher<RouterInfo>
      routeHandlesCommandDispatcher_;
  mutable detail::RouteCommandDispatcher<RouterInfo> routeCommandDispatcher_;
  mutable detail::GetBucketCommandDispatcher<RouterInfo>
      getBucketCommandDispatcher_;

  ServiceInfoImpl(
      Proxy<RouterInfo>& proxy,
      const ProxyConfig<RouterInfo>& config);

  void handleRequest(
      folly::StringPiece req,
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx) const;

  void handleRouteCommand(
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
      const std::vector<folly::StringPiece>& args) const;

  void handleGetBucketCommand(
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
      const std::vector<folly::StringPiece>& args) const;
};

/* Must be here since unique_ptr destructor needs to know complete
   ServiceInfoImpl type */
template <class RouterInfo>
ServiceInfo<RouterInfo>::~ServiceInfo() {}

template <class RouterInfo>
ServiceInfo<RouterInfo>::ServiceInfo(
    Proxy<RouterInfo>& proxy,
    const ProxyConfig<RouterInfo>& config)
    : impl_(std::make_unique<ServiceInfoImpl>(proxy, config)) {}

template <class RouterInfo>
ServiceInfo<RouterInfo>::ServiceInfoImpl::ServiceInfoImpl(
    Proxy<RouterInfo>& proxy,
    const ProxyConfig<RouterInfo>& config)
    : proxy_(proxy), proxyRoute_(config.proxyRoute()) {
  commands_.emplace(
      "version", [](const std::vector<folly::StringPiece>& /* args */) {
        return MCROUTER_PACKAGE_STRING;
      });

  commands_.emplace(
      "config_age",
      [&proxy](const std::vector<folly::StringPiece>& /* args */) {
        /* capturing this and accessing proxy_ crashes gcc-4.7 */
        return std::to_string(proxy.stats().getConfigAge(time(nullptr)));
      });

  commands_.emplace(
      "config_file", [this](const std::vector<folly::StringPiece>& /* args */) {
        folly::StringPiece configStr = proxy_.router().opts().config;
        if (configStr.startsWith(ConfigApi::kFilePrefix)) {
          configStr.removePrefix(ConfigApi::kFilePrefix);
          return configStr.str();
        }

        if (proxy_.router().opts().config_file.empty()) {
          throw std::runtime_error("no config file found!");
        }

        return proxy_.router().opts().config_file;
      });

  commands_.emplace(
      "options", [this](const std::vector<folly::StringPiece>& args) {
        if (args.size() > 1) {
          throw std::runtime_error("options: 0 or 1 args expected");
        }

        auto optDict = proxy_.router().getStartupOpts();

        if (args.size() == 1) {
          auto it = optDict.find(args[0].str());
          if (it == optDict.end()) {
            throw std::runtime_error(
                "options: option " + args[0].str() + " not found");
          }
          return it->second;
        }

        // Print all options in order listed in the file
        auto optData = McrouterOptions::getOptionData();
        auto startupOpts = McrouterStandaloneOptions::getOptionData();
        optData.insert(optData.end(), startupOpts.begin(), startupOpts.end());
        std::string str;
        for (auto& opt : optData) {
          if (optDict.find(opt.name) != optDict.end()) {
            str.append(opt.name + " " + optDict[opt.name] + "\n");
          }
        }
        return str;
      });

  /*
    This is a special case and handled separately below

  {"route", ...
  },

  */

  commands_.emplace(
      "route_handles", [this](const std::vector<folly::StringPiece>& args) {
        if (args.size() != 2) {
          throw std::runtime_error("route_handles: 2 args expected");
        }
        auto requestName = args[0];
        auto key = args[1];

        auto typeId = carbon::getTypeIdByName(
            requestName, typename RouterInfo::RoutableRequests());

        std::string res;
        if (!routeHandlesCommandDispatcher_.dispatch(
                typeId, key, proxyRoute_, res)) {
          throw std::runtime_error(
              folly::sformat("route: unknown request {}", requestName));
        }
        return res;
      });

  commands_.emplace(
      "config_md5_digest",
      [&config](const std::vector<folly::StringPiece>& /* args */) {
        if (config.getConfigMd5Digest().empty()) {
          throw std::runtime_error("no config md5 digest found!");
        }
        return config.getConfigMd5Digest();
      });

  commands_.emplace(
      "config_sources_info",
      [this](const std::vector<folly::StringPiece>& /* args */) {
        auto configInfo = proxy_.router().configApi().getConfigSourcesInfo();
        return toPrettySortedJson(configInfo);
      });

  commands_.emplace(
      "preprocessed_config",
      [this](const std::vector<folly::StringPiece>& /* args */) {
        std::string confFile;
        std::string path;
        if (!proxy_.router().configApi().getConfigFile(confFile, path)) {
          throw std::runtime_error("Can not load config from " + path);
        }
        ProxyConfigBuilder builder(
            proxy_.router().opts(),
            proxy_.router().configApi(),
            confFile,
            RouterInfo::name);
        return toPrettySortedJson(builder.preprocessedConfig());
      });

  commands_.emplace(
      "hostid", [](const std::vector<folly::StringPiece>& /* args */) {
        return folly::to<std::string>(globals::hostid());
      });

  commands_.emplace(
      "verbosity", [](const std::vector<folly::StringPiece>& args) {
        if (args.size() == 1) {
          auto before = FLAGS_v;
          FLAGS_v = folly::to<int>(args[0]);
          return folly::sformat("{} -> {}", before, FLAGS_v);
        } else if (args.empty()) {
          return folly::to<std::string>(FLAGS_v);
        }
        throw std::runtime_error(
            "expected at most 1 argument, got " +
            folly::to<std::string>(args.size()));
      });

  commands_.emplace(
      "partial_config_enabled_pools",
      [&config](const std::vector<folly::StringPiece>& /* args */) {
        auto partialConfigs = config.getPartialConfigs();
        folly::dynamic result = folly::dynamic::object;
        for (const auto& partialConfig : partialConfigs) {
          folly::dynamic poolsJson = folly::dynamic::object;
          for (const auto& p : partialConfig.second.second) {
            poolsJson.update(
                facebook::memcache::mcrouter::
                    getConfigJsonFromCommonAccessPointAttributes(p.first));
          }
          result[partialConfig.first] = poolsJson;
        }
        return toPrettySortedJson(result);
      });
  commands_.emplace(
      "test_replace_ap", [this](const std::vector<folly::StringPiece>& args) {
        auto& configApi = proxy_.router().configApi();
        if (args.size() != 3) {
          return "Error";
        }
        auto arg1 = args[1].str();
        auto arg2 = args[2].str();
        auto idx1 = arg1.find(":");
        auto idx2 = arg2.find(":");
        if (idx1 == std::string::npos || idx2 == std::string::npos) {
          return "Error";
        }
        // As a safety check, allow this only for loopback addresses
        auto host1 = arg1.substr(0, idx1);
        auto host2 = arg2.substr(0, idx2);
        if (!folly::IPAddress(host1).isLoopback() ||
            !folly::IPAddress(host2).isLoopback()) {
          return "Error";
        }
        facebook::memcache::mcrouter::ConfigApi::PartialUpdate update = {
            args[0].str(), arg1, arg2, 1, 1, 11111, "", 1};
        configApi.addPartialUpdateForTest(update);
        return "Success";
      });

  commands_.emplace(
      "pools", [&config](const std::vector<folly::StringPiece>& /* args */) {
        folly::dynamic result = folly::dynamic::object;
        auto pools = config.getPools();
        for (const auto& pool : pools) {
          folly::dynamic servers = folly::dynamic::array;
          const auto& poolServers = pool.second;
          for (auto pdstn : poolServers) {
            servers.push_back(pdstn->routeName());
          }
          result[pool.first] = servers;
        }
        return toPrettySortedJson(result);
      });

  commands_.emplace(
      "failure_domains", [this](const std::vector<folly::StringPiece>& args) {
        if (args.size() != 1) {
          throw std::runtime_error("failure_domains: 1 arg expected");
        }
        auto& configApi = proxy_.router().configApi();
        auto& accessPoints = proxy_.getConfigUnsafe()->getAccessPoints();
        folly::dynamic result = folly::dynamic::object;
        auto it = accessPoints.find(args[0].str());
        if (it != accessPoints.end()) {
          for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            auto ap = *it2;
            auto failureDomain = ap->getFailureDomain();
            result[ap->toHostPortString()] = fmt::format(
                "{}({})",
                failureDomain,
                configApi.getFailureDomainStr(failureDomain));
          }
        }

        return toPrettySortedJson(result);
      });

  commands_.emplace(
      "global_params",
      [this](const std::vector<folly::StringPiece>& /* args */) {
        folly::dynamic result = folly::dynamic::object;
        auto globalParams = ProxyConfigBuilder::buildGlobalParams(
            proxy_.router().opts(), RouterInfo::name);
        for (const auto& [key, value] : globalParams) {
          result[key] = value;
        }
        return toPrettySortedJson(result);
      });
}

template <class RouterInfo>
void ServiceInfo<RouterInfo>::ServiceInfoImpl::handleRequest(
    folly::StringPiece key,
    const std::shared_ptr<
        ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx) const {
  auto p = key.find('(');
  auto cmd = key;
  folly::StringPiece argsStr(key.end(), key.end());
  if (p != folly::StringPiece::npos && key.back() == ')') {
    assert(key.size() - p >= 2);
    cmd = folly::StringPiece(key.begin(), key.begin() + p);
    argsStr =
        folly::StringPiece(key.begin() + p + 1, key.begin() + key.size() - 1);
  }
  std::vector<folly::StringPiece> args;
  if (!argsStr.empty()) {
    // Handle keys with commas correctly for route and route_handles subcommands
    if (cmd.startsWith("route")) {
      const auto pos = argsStr.find(',');
      if (pos != std::string::npos) {
        args.emplace_back(argsStr.data(), pos);
        args.emplace_back(argsStr.data() + pos + 1, argsStr.size() - pos - 1);
      }
    } else {
      folly::split(',', argsStr, args);
    }
  }

  std::string replyStr;
  try {
    if (cmd == "route") {
      /* Route is a special case since it involves background requests */
      handleRouteCommand(ctx, args);
      return;
    }

    if (cmd == "get_bucket") {
      handleGetBucketCommand(ctx, args);
      return;
    }

    auto it = commands_.find(cmd.str());
    if (it == commands_.end()) {
      throw std::runtime_error("unknown command: " + cmd.str());
    }
    replyStr = it->second(args);
    if (!replyStr.empty() && replyStr.back() == '\n') {
      replyStr = replyStr.substr(0, replyStr.size() - 1);
    }
  } catch (const std::exception& e) {
    replyStr = std::string("ERROR: ") + e.what();
  }
  ReplyT<ServiceInfoRequest> reply(carbon::Result::FOUND);
  reply.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, replyStr);
  ctx->sendReply(std::move(reply));
}

template <class RouterInfo>
void ServiceInfo<RouterInfo>::ServiceInfoImpl::handleRouteCommand(
    const std::shared_ptr<
        ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
    const std::vector<folly::StringPiece>& args) const {
  if (args.size() != 2) {
    throw std::runtime_error("route: 2 args expected");
  }
  auto requestName = args[0];
  auto key = args[1];

  auto typeId = carbon::getTypeIdByName(
      requestName, typename RouterInfo::RoutableRequests());

  if (!routeCommandDispatcher_.dispatch(
          typeId, ctx, key, proxy_, proxyRoute_)) {
    throw std::runtime_error(
        folly::sformat("route: unknown request {}", requestName));
  }
}

template <class RouterInfo>
void ServiceInfo<RouterInfo>::ServiceInfoImpl::handleGetBucketCommand(
    const std::shared_ptr<
        ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
    const std::vector<folly::StringPiece>& args) const {
  if (args.size() != 1) {
    throw std::runtime_error("get_bucket: 1 arg (key) expected");
  }
  auto key = args[0];

  std::string res;
  if (!getBucketCommandDispatcher_.dispatch(
          facebook::memcache::McGetRequest::typeId,
          ctx,
          key,
          proxy_,
          proxyRoute_)) {
    throw std::runtime_error(
        folly::sformat("get_bucket: couldn't find bucket for key {}", key));
  }
}

template <class RouterInfo>
void ServiceInfo<RouterInfo>::handleRequest(
    folly::StringPiece key,
    const std::shared_ptr<
        ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx) const {
  impl_->handleRequest(key, ctx);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
