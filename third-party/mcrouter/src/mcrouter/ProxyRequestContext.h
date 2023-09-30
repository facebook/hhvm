/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>

#include <folly/Range.h>
#include <folly/fibers/FiberManager.h>

#include "mcrouter/ProxyRequestPriority.h"
#include "mcrouter/config-impl.h"
#include "mcrouter/lib/PoolContext.h"
#include "mcrouter/lib/carbon/Result.h"

namespace facebook {
namespace memcache {

struct AccessPoint;

namespace mcrouter {

template <class RouterInfo>
class Proxy;
template <class RouterInfo>
class ProxyRoute;

class ProxyBase;
class CarbonRouterClientBase;
class ShardSplitter;

/**
 * This object is alive for the duration of user's request,
 * including any subrequests that might have been sent out.
 *
 * It starts it's life under a unique_ptr outside of proxy threads.
 * When handed off to a proxy thread and ready to execute,
 * we save the current configuration and convert it to shared
 * ownership.
 *
 * Records collected stats on destruction.
 */
class ProxyRequestContext {
 public:
  using ClientCallback =
      std::function<void(const PoolContext&, const AccessPoint&)>;
  using ShardSplitCallback = std::function<void(const ShardSplitter&, bool)>;
  using BucketizationCallback =
      std::function<void(std::string, const uint64_t, const std::string&)>;

  virtual ~ProxyRequestContext();

  ProxyBase& proxy() const {
    return proxyBase_;
  }

  bool recording() const noexcept {
    return recording_;
  }

  bool recordingBucketData() const noexcept {
    return recording_ && recordingState_->bucketizationCallback;
  }

  void recordDestination(const PoolContext& poolContext, const AccessPoint& ap)
      const {
    if (recording_ && recordingState_->clientCallback) {
      recordingState_->clientCallback(poolContext, ap);
    }
  }

  void recordShardSplitter(const ShardSplitter& splitter, bool isShadow) const {
    if (recording_ && recordingState_->shardSplitCallback) {
      recordingState_->shardSplitCallback(splitter, isShadow);
    }
  }

  void recordBucketizationData(
      std::string key,
      const uint64_t bucketId,
      const std::string& keyspace) const {
    if (recording_ && recordingState_->bucketizationCallback) {
      recordingState_->bucketizationCallback(
          std::move(key), bucketId, keyspace);
    }
  }

  uint64_t senderId() const;

  void setSenderIdForTest(uint64_t id);

  bool failoverDisabled() const {
    return failoverDisabled_;
  }

  void setPoolStatsIndex(int32_t index) {
    if (poolStatIndex_ == -1) {
      poolStatIndex_ = index;
    }
  }

  ProxyRequestPriority priority() const {
    return priority_;
  }

  /**
   * Continues processing current request.
   * Should be called only from the attached proxy thread.
   */
  virtual void startProcessing() {
    throw std::logic_error(
        "Calling startProcessing on an incomplete instance "
        "of ProxyRequestContext");
  }

  const std::string& userIpAddress() const noexcept {
    return userIpAddr_;
  }

  void setUserIpAddress(folly::StringPiece newAddr) noexcept {
    userIpAddr_ = newAddr.str();
  }

  bool isProcessing() const {
    return processing_;
  }
  void markAsProcessing() {
    processing_ = true;
  }

  void setRequester(std::shared_ptr<CarbonRouterClientBase> requester) {
    requester_ = std::move(requester);
  }

  void setFinalResult(carbon::Result result) {
    finalResult_ = result;
  }

  carbon::Result finalResult() const {
    return finalResult_;
  }

  /**
   * Set preprocess function.
   */
  void setPreprocessFunction(std::function<void()>&& f) {
    mcrouterPreprocess_ = f;
  }

  /**
   * Run preprocess function.
   */
  void runPreprocessFunction() const {
    if (FOLLY_UNLIKELY(mcrouterPreprocess_ != nullptr)) {
      mcrouterPreprocess_();
    }
  }

  /**
   * Set RequestContextScopeGuard to create folly::RequestContext.
   */
  void setRequestContextScopeGuard(
      std::unique_ptr<folly::ShallowCopyRequestContextScopeGuard> guard) {
    reqContextScopeGuard_ = std::move(guard);
  }

  /**
   * Destroy RequestContextScopeGuard to destroy folly::RequestContext.
   */
  void destroyRequestContextScopeGuard() {
    reqContextScopeGuard_.reset();
  }

  void setRoutingHint(uint64_t hint) {
    routingHint_ = hint;
  }

  uint64_t getRoutingHint() const {
    return routingHint_;
  }

 protected:
  // Keep on first cacheline. Used by ProxyRequestContextTyped
  const void* ptr_{nullptr};
  carbon::Result finalResult_{carbon::Result::UNKNOWN};
  int32_t poolStatIndex_{-1};
  bool replied_{false};

  ProxyRequestContext(
      ProxyBase& pr,
      ProxyRequestPriority priority__,
      const void* ptr = nullptr);

  enum RecordingT { Recording };
  ProxyRequestContext(
      RecordingT,
      ProxyBase& pr,
      ClientCallback clientCallback,
      ShardSplitCallback shardSplitCallback,
      BucketizationCallback bucketizationCallback);

 private:
  ProxyBase& proxyBase_;

  std::shared_ptr<CarbonRouterClientBase> requester_;

  struct RecordingState {
    ClientCallback clientCallback;
    ShardSplitCallback shardSplitCallback;
    BucketizationCallback bucketizationCallback;
  };

  union {
    void* context_{nullptr};
    std::unique_ptr<RecordingState> recordingState_;
  };

  uint64_t senderIdForTest_{0};

  std::string userIpAddr_;

  ProxyRequestPriority priority_{ProxyRequestPriority::kCritical};

  bool failoverDisabled_{false};
  /** If true, this is currently being processed by a proxy and
      we want to notify we're done on destruction. */
  bool processing_{false};
  bool recording_{false};

  /** A host hint to be passed to the routing layer, if thread affinity is
      enabled. This avoids having to perform host selection twice in the routing
      layer. */
  uint64_t routingHint_{0};

  /**
   * Functions to be executed before actual processing code.
   */
  std::function<void()> mcrouterPreprocess_{nullptr};
  std::unique_ptr<folly::ShallowCopyRequestContextScopeGuard>
      reqContextScopeGuard_{nullptr};

  ProxyRequestContext(const ProxyRequestContext&) = delete;
  ProxyRequestContext(ProxyRequestContext&&) noexcept = delete;
  ProxyRequestContext& operator=(const ProxyRequestContext&) = delete;
  ProxyRequestContext& operator=(ProxyRequestContext&&) = delete;

 private:
  friend class ProxyBase;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
