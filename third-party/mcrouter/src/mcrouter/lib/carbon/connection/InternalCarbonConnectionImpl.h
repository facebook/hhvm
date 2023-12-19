/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <mcrouter/McrouterClient.h>
#include <mcrouter/lib/CacheClientStats.h>
#include <mcrouter/lib/carbon/connection/CarbonConnectionUtil.h>
#include <mcrouter/options.h>

namespace carbon {

struct InternalCarbonConnectionOptions {
  InternalCarbonConnectionOptions() = default;
  size_t maxOutstanding{1024};
  size_t maxOutstandingError{false};
  std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreads{nullptr};
};

template <class If>
class InternalCarbonConnectionImpl {
 public:
  using RecreateFunc = std::function<std::unique_ptr<If>()>;

  explicit InternalCarbonConnectionImpl(
      const std::string& persistenceId,
      const InternalCarbonConnectionOptions& options =
          InternalCarbonConnectionOptions(),
      RecreateFunc recreateFunc = nullptr)
      : recreateFunc_(std::move(recreateFunc)) {
    init(
        facebook::memcache::mcrouter::CarbonRouterInstance<
            typename If::RouterInfo>::get(persistenceId),
        options);
  }

  explicit InternalCarbonConnectionImpl(
      const std::string& persistenceId,
      const facebook::memcache::McrouterOptions& mcrouterOptions,
      const InternalCarbonConnectionOptions& options =
          InternalCarbonConnectionOptions(),
      RecreateFunc recreateFunc = nullptr)
      : recreateFunc_(std::move(recreateFunc)) {
    init(
        facebook::memcache::mcrouter::
            CarbonRouterInstance<typename If::RouterInfo>::init(
                persistenceId, mcrouterOptions, options.ioThreads),
        options);
  }

  explicit InternalCarbonConnectionImpl(
      facebook::memcache::mcrouter::CarbonRouterInstance<
          typename If::RouterInfo>& router,
      const InternalCarbonConnectionOptions& options =
          InternalCarbonConnectionOptions(),
      RecreateFunc recreateFunc = nullptr)
      : recreateFunc_(std::move(recreateFunc)) {
    init(&router, options);
  }

  explicit InternalCarbonConnectionImpl(
      const facebook::memcache::McrouterOptions& mcrouterOptions,
      const InternalCarbonConnectionOptions& options =
          InternalCarbonConnectionOptions(),
      RecreateFunc recreateFunc = nullptr)
      : recreateFunc_(std::move(recreateFunc)) {
    transientRouter_ = facebook::memcache::mcrouter::CarbonRouterInstance<
        typename If::RouterInfo>::create(mcrouterOptions.clone());
    init(transientRouter_.get(), options);
  }

  facebook::memcache::CacheClientCounters getStatCounters() const noexcept {
    return client_->getStatCounters();
  }

  bool healthCheck() const {
    return true;
  }

  std::unordered_map<std::string, std::string> getConfigOptions() const {
    return router_->opts().toDict();
  }

  template <class Impl>
  std::unique_ptr<If> recreate() {
    return recreateFunc_();
  }

  template <class Request>
  void sendRequestOne(const Request& req, RequestCb<Request> cb) {
    if (!client_->send(req, std::move(cb))) {
      throw CarbonConnectionRecreateException(
          "mcrouter instance was destroyed");
    }
  }

  template <class Request>
  void sendRequestMulti(
      std::vector<std::reference_wrapper<const Request>>&& reqs,
      RequestCb<Request> cb) {
    if (!client_->send(reqs.begin(), reqs.end(), std::move(cb))) {
      throw CarbonConnectionRecreateException(
          "mcrouter instance was destroyed");
    }
  }

 private:
  typename facebook::memcache::mcrouter::CarbonRouterClient<
      typename If::RouterInfo>::Pointer client_;
  facebook::memcache::mcrouter::CarbonRouterInstance<typename If::RouterInfo>*
      router_;
  std::shared_ptr<facebook::memcache::mcrouter::CarbonRouterInstance<
      typename If::RouterInfo>>
      transientRouter_;
  RecreateFunc recreateFunc_;

  void init(
      facebook::memcache::mcrouter::CarbonRouterInstance<
          typename If::RouterInfo>* router,
      const InternalCarbonConnectionOptions& options) {
    router_ = router;
    if (!router_) {
      throw CarbonConnectionException("Failed to initialize router");
    }
    try {
      if (router_->opts().force_same_thread) {
        client_ = router_->createSameThreadClient(options.maxOutstanding);
      } else {
        client_ = router_->createClient(
            options.maxOutstanding, options.maxOutstandingError);
      }
    } catch (const std::runtime_error& e) {
      throw CarbonConnectionException(
          std::string("Failed to initialize router client: ") + e.what());
    }
  }
};
} // namespace carbon
