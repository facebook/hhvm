/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <mutex>
#include <unordered_map>

#include <folly/MapUtil.h>
#include <folly/Range.h>
#include <folly/Singleton.h>

#include "mcrouter/CarbonRouterFactory.h"
#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/routes/CarbonLookasideRoute.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

class CarbonLookasideManager {
 public:
  std::shared_ptr<CarbonRouterInstance<MemcacheRouterInfo>>
  createCarbonLookasideRouter(
      const std::string& persistenceId,
      folly::StringPiece flavorUri,
      std::unordered_map<std::string, std::string> optionOverrides) {
    std::lock_guard<std::mutex> ilg(initMutex_);
    std::shared_ptr<CarbonRouterInstance<MemcacheRouterInfo>> mcrouter =
        folly::get_default(mcroutersLookaside_, persistenceId).lock();
    if (!mcrouter) {
      const std::string kServiceName = "service_name";
      if (optionOverrides.find(kServiceName) == optionOverrides.end()) {
        optionOverrides.emplace(kServiceName, "carbon");
      }
      try {
        mcrouter = createRouterFromFlavor<MemcacheRouterInfo>(
            flavorUri, optionOverrides);
        if (mcrouter) {
          mcroutersLookaside_[persistenceId] = mcrouter;
        }
      } catch (const std::exception& ex) {
        mcrouter = nullptr;
        LOG(ERROR) << "Error creating mcrouter for lookaside route. Exception: "
                   << ex.what();
      }
    }
    return mcrouter;
  }

 private:
  std::mutex initMutex_;
  std::unordered_map<
      std::string,
      std::weak_ptr<CarbonRouterInstance<MemcacheRouterInfo>>>
      mcroutersLookaside_;
};

folly::Singleton<CarbonLookasideManager> gCarbonLookasideManager;
} // namespace

std::shared_ptr<CarbonRouterInstance<MemcacheRouterInfo>>
createCarbonLookasideRouter(
    const std::string& persistenceId,
    folly::StringPiece flavorUri,
    std::unordered_map<std::string, std::string> optionOverrides) {
  if (auto manager = gCarbonLookasideManager.try_get()) {
    return manager->createCarbonLookasideRouter(
        persistenceId, flavorUri, optionOverrides);
  }
  return nullptr;
}

LeaseSettings parseLeaseSettings(const folly::dynamic& json) {
  LeaseSettings leaseSettings;
  if (auto jLeases = json.get_ptr("lease_settings")) {
    checkLogic(
        jLeases->isObject(),
        "CarbonLookasideRoute: 'lease_settings' is not an object");
    if (auto jEnableLeases = jLeases->get_ptr("enable_leases")) {
      checkLogic(
          jEnableLeases->isBool(),
          "CarbonLookasideRoute: 'enable_leases' is not bool");
      leaseSettings.enableLeases = jEnableLeases->getBool();
    }
    if (auto jInitialWait = jLeases->get_ptr("initial_wait_interval_ms")) {
      checkLogic(
          jInitialWait->isInt(),
          "CarbonLookasideRoute: 'initial_wait_interval_ms' is not an int");
      leaseSettings.initialWaitMs = jInitialWait->getInt();
      checkLogic(
          leaseSettings.initialWaitMs >= 0,
          "CarbonLookasideRoute: 'initial_wait_interval_ms' must be >= 0");
    }
    if (auto jMaxWait = jLeases->get_ptr("max_wait_interval_ms")) {
      checkLogic(
          jMaxWait->isInt(),
          "CarbonLookasideRoute: 'max_wait_interval_ms' is not an int");
      leaseSettings.maxWaitMs = jMaxWait->getInt();
      checkLogic(
          leaseSettings.maxWaitMs >= 0,
          "CarbonLookasideRoute: 'max_wait_interval_ms' must be >= 0");
    }
    if (auto jNumRetries = jLeases->get_ptr("num_retries")) {
      checkLogic(
          jNumRetries->isInt(),
          "CarbonLookasideRoute: 'num_retries' is not an int");
      leaseSettings.numRetries = jNumRetries->getInt();
      checkLogic(
          leaseSettings.numRetries >= 0,
          "CarbonLookasideRoute: 'num_retries' must be >= 0");
    }
  }
  return leaseSettings;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
