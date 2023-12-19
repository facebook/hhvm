/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <mutex>

#include <folly/Range.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/EventBase.h>
#include <memory>

#include "mcrouter/CarbonRouterInstanceBase.h"
namespace facebook {
namespace memcache {
namespace mcrouter {
template <class RouterInfo>
class CarbonRouterInstance;

namespace detail {
class McrouterManager {
 public:
  McrouterManager();

  ~McrouterManager();

  void freeAllMcrouters();

  template <class RouterInfo>
  CarbonRouterInstance<RouterInfo>* mcrouterGetCreate(
      folly::StringPiece persistenceId,
      const McrouterOptions& options,
      std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool = nullptr) {
    std::shared_ptr<CarbonRouterInstanceBase> mcrouterBase;

    {
      std::lock_guard<std::mutex> lg(mutex_);
      mcrouterBase = folly::get_default(mcrouters_, persistenceId.str());
    }
    if (!mcrouterBase) {
      std::lock_guard<std::mutex> ilg(initMutex_);
      {
        std::lock_guard<std::mutex> lg(mutex_);
        mcrouterBase = folly::get_default(mcrouters_, persistenceId.str());
      }
      if (!mcrouterBase) {
        std::shared_ptr<CarbonRouterInstance<RouterInfo>> mcrouter =
            CarbonRouterInstance<RouterInfo>::create(
                options.clone(), std::move(ioThreadPool));
        if (mcrouter) {
          std::lock_guard<std::mutex> lg(mutex_);
          mcrouters_[persistenceId.str()] = mcrouter;
          return mcrouter.get();
        }
      }
    }
    auto ret =
        dynamic_cast<CarbonRouterInstance<RouterInfo>*>(mcrouterBase.get());
    if (!mcrouterBase) {
      MC_LOG_FAILURE(
          options,
          failure::Category::kBrokenLogic,
          folly::sformat(
              "mcrouterGetCreate failed to create mcrouter instance with router info {} for persistentId {}",
              RouterInfo::name,
              persistenceId));
    } else if (ret == nullptr) {
      MC_LOG_FAILURE(
          options,
          failure::Category::kBrokenLogic,
          folly::sformat(
              "mcrouterGetCreate failed to cast mcrouter instance to router info {} for persistentId {}",
              RouterInfo::name,
              persistenceId));
    }
    return ret;
  }

  template <class RouterInfo>
  CarbonRouterInstance<RouterInfo>* mcrouterGet(
      folly::StringPiece persistenceId) {
    std::lock_guard<std::mutex> lg(mutex_);
    auto mcrouterBase =
        folly::get_default(mcrouters_, persistenceId.str(), nullptr).get();
    return dynamic_cast<CarbonRouterInstance<RouterInfo>*>(mcrouterBase);
  }

  static std::shared_ptr<McrouterManager> getSingletonInstance();

 private:
  std::unordered_map<std::string, std::shared_ptr<CarbonRouterInstanceBase>>
      mcrouters_;
  // protects mcrouters_
  std::mutex mutex_;
  // initMutex_ must not be taken under mutex_, otherwise deadlock is possible
  std::mutex initMutex_;
};
} // namespace detail
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
