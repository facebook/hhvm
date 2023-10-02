/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <bitset>
#include <map>
#include <memory>
#include <utility>

#include <folly/Range.h>
#include <folly/ScopeGuard.h>
#include <folly/container/F14Map.h>
#include <folly/fibers/FiberManager.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Aligned.h>

#include "mcrouter/lib/network/ServerLoad.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
class ProxyRequestContextWithInfo;

class RequestClass {
 public:
  static const RequestClass kFailover;
  static const RequestClass kShadow;

  constexpr RequestClass() {}

  void add(RequestClass rc) {
    mask_ |= rc.mask_;
  }

  bool is(RequestClass rc) const {
    return (mask_ & rc.mask_) == rc.mask_;
  }

  bool isNormal() const {
    return mask_ == 0;
  }

  const char* toString() const;

 private:
  explicit constexpr RequestClass(uint32_t value) : mask_(value) {}

  uint32_t mask_{0};
};

using ExtraDataMap = folly::F14FastMap<std::string, std::string>;
using ExtraDataCallbackT = std::function<ExtraDataMap()>;
using AxonProxyWriteFn = std::function<
    bool(uint64_t, folly::F14FastMap<std::string, std::string>&&)>;

struct AxonContext {
  bool fallbackAsynclog{false};
  bool allDelete{false};
  AxonProxyWriteFn writeProxyFn;
  std::string defaultRegionFilter;
  std::string poolFilter;
};

template <class RouterInfo>
class fiber_local {
 private:
  enum FeatureFlag : size_t {
    FAILOVER_DISABLED,
    THRIFT_SERVER_LOAD_ENABLED,
    NUM_FLAGS
  };

  struct alignas(folly::cacheline_align_v) McrouterFiberContext {
    std::shared_ptr<ProxyRequestContextWithInfo<RouterInfo>> sharedCtx;
    std::bitset<NUM_FLAGS> featureFlags;
    int32_t selectedIndex{-1};
    uint32_t failoverCount{0};
    std::optional<uint64_t> bucketId;
    std::optional<std::string> distributionTargetRegion;
    RequestClass requestClass;
    folly::StringPiece asynclogName;
    int64_t networkTransportTimeUs{0};
    ServerLoad load{0};
    std::vector<ExtraDataCallbackT> extraDataCallbacks;
    std::shared_ptr<AxonContext> axonCtx{nullptr};
    int64_t accumulatedBeforeReqInjectedLatencyUs{0};
    int64_t accumulatedAfterReqInjectedLatencyUs{0};
  };

  static auto makeGuardHelperBase(McrouterFiberContext&& tmp) {
    return folly::makeGuard([tmp]() mutable {
      folly::fibers::local<McrouterFiberContext>() = std::move(tmp);
    });
  }

  static auto makeGuardHelperCopy() {
    auto tmp = folly::fibers::local<McrouterFiberContext>();
    return makeGuardHelperBase(std::move(tmp));
  }

  static auto makeGuardHelperReset() {
    auto tmp = std::move(folly::fibers::local<McrouterFiberContext>());
    folly::fibers::local<McrouterFiberContext>() = McrouterFiberContext();
    return makeGuardHelperBase(std::move(tmp));
  }

 public:
  using ContextTypeTag = folly::fibers::LocalType<McrouterFiberContext>;

  /**
   * Clear all locals, run `f`, restore locals
   */
  template <class F>
  static typename std::result_of<F()>::type runWithoutLocals(F&& f) {
    auto guard = makeGuardHelperReset();

    return f();
  }

  /**
   * Copy all locals, run `f`, restore locals
   */
  template <class F>
  static typename std::result_of<F()>::type runWithLocals(F&& f) {
    auto guard = makeGuardHelperCopy();

    return f();
  }

  /**
   * Update ProxyRequestContextWithInfo for current fiber (thread, if we're not
   * on fiber)
   */
  static void setSharedCtx(
      std::shared_ptr<ProxyRequestContextWithInfo<RouterInfo>> ctx) {
    folly::fibers::local<McrouterFiberContext>().sharedCtx = std::move(ctx);
  }

  /**
   * Get ProxyRequestContextWithInfo of current fiber (thread, if we're not on
   * fiber)
   */
  static const std::shared_ptr<ProxyRequestContextWithInfo<RouterInfo>>&
  getSharedCtx() {
    return folly::fibers::local<McrouterFiberContext>().sharedCtx;
  }

  /**
   * Get ProxyRequestContextWithInfo of current fiber (thread, if we're not on
   * fiber).
   * Can only be called from the RouteHandle's traverse() function.  Since
   * traverse() is not guaranteed to be called from the proxy thread, only
   * methods that access proxy/mcrouter in threadsafe way are allowed
   * to be called on the context.
   */
  static const ProxyRequestContextWithInfo<RouterInfo>* getTraverseCtx() {
    return folly::fibers::local<McrouterFiberContext>().sharedCtx.get();
  }

  /**
   * Add a RequestClass for current fiber (thread, if we're not on fiber)
   */
  static void addRequestClass(RequestClass value) {
    folly::fibers::local<McrouterFiberContext>().requestClass.add(value);
  }

  /**
   * Get RequestClass of current fiber (thread, if we're not on fiber)
   */
  static RequestClass getRequestClass() {
    return folly::fibers::local<McrouterFiberContext>().requestClass;
  }

  /**
   * Update AsynclogName for current fiber (thread, if we're not on fiber)
   */
  static void setAsynclogName(folly::StringPiece value) {
    folly::fibers::local<McrouterFiberContext>().asynclogName = value;
  }

  /**
   * Clear AsynclogName for current fiber (thread, if we're not on fiber)
   */
  static void clearAsynclogName() {
    setAsynclogName("");
  }

  /**
   * Get asynclog name of current fiber (thread, if we're not on fiber)
   */
  static folly::StringPiece getAsynclogName() {
    return folly::fibers::local<McrouterFiberContext>().asynclogName;
  }

  /**
   * Increment failover count for current fiber (thread, if we're not on fiber)
   * and return the new value
   */
  static uint32_t incFailoverCount() {
    folly::fibers::local<McrouterFiberContext>().failoverCount += 1;
    return folly::fibers::local<McrouterFiberContext>().failoverCount;
  }

  /**
   * Get failover count of current fiber (thread, if we're not on fiber)
   */
  static uint32_t getFailoverCount() {
    return folly::fibers::local<McrouterFiberContext>().failoverCount;
  }

  /**
   * Accumulate latency injected before request for current fiber and return the
   * new value
   */
  static int64_t accumulateBeforeReqInjectedLatencyUs(
      uint64_t additionalInjectedUs) {
    folly::fibers::local<McrouterFiberContext>()
        .accumulatedBeforeReqInjectedLatencyUs += additionalInjectedUs;
    return folly::fibers::local<McrouterFiberContext>()
        .accumulatedBeforeReqInjectedLatencyUs;
  }

  /**
   * Get accumulated latency injected for current fiber before request
   */
  static int64_t getAccumulatedInjectedBeforeReqLatencyUs() {
    return folly::fibers::local<McrouterFiberContext>()
        .accumulatedBeforeReqInjectedLatencyUs;
  }

  /**
   * Accumulate latency injected after request for current fiber and return the
   * new value
   */
  static int64_t accumulateAfterReqInjectedLatencyUs(
      uint64_t additionalInjectedUs) {
    folly::fibers::local<McrouterFiberContext>()
        .accumulatedAfterReqInjectedLatencyUs += additionalInjectedUs;
    return folly::fibers::local<McrouterFiberContext>()
        .accumulatedAfterReqInjectedLatencyUs;
  }

  /**
   * Get accumulated latency injected for current fiber after request
   */
  static int64_t getAccumulatedInjectedAfterReqLatencyUs() {
    return folly::fibers::local<McrouterFiberContext>()
        .accumulatedAfterReqInjectedLatencyUs;
  }

  /**
   * Set selected index for normal reply from the target_ list.
   * it will be used for the iterator to avoid getting duplicates
   */
  static void setSelectedIndex(int32_t value) {
    folly::fibers::local<McrouterFiberContext>().selectedIndex = value;
  }

  /**
   * Get selected index for normal reply
   */
  static int32_t getSelectedIndex() {
    return folly::fibers::local<McrouterFiberContext>().selectedIndex;
  }

  /**
   * Set failover disabled flag for current fiber (thread, if we're not on
   * fiber)
   */
  static void setFailoverDisabled(bool value) {
    folly::fibers::local<McrouterFiberContext>().featureFlags.set(
        FeatureFlag::FAILOVER_DISABLED, value);
  }

  /**
   * Get failover disabled tag of current fiber (thread, if we're not on fiber)
   */
  static bool getFailoverDisabled() {
    return folly::fibers::local<McrouterFiberContext>().featureFlags.test(
        FeatureFlag::FAILOVER_DISABLED);
  }

  static void setServerLoad(ServerLoad load) {
    folly::fibers::local<McrouterFiberContext>().load = load;
  }

  static ServerLoad getServerLoad() {
    return folly::fibers::local<McrouterFiberContext>().load;
  }

  static void incNetworkTransportTimeBy(int64_t duration_us) {
    folly::fibers::local<McrouterFiberContext>().networkTransportTimeUs +=
        duration_us;
  }

  static int64_t getNetworkTransportTimeUs() {
    return folly::fibers::local<McrouterFiberContext>().networkTransportTimeUs;
  }

  static void setThriftServerLoadEnabled(bool value) {
    folly::fibers::local<McrouterFiberContext>().featureFlags.set(
        FeatureFlag::THRIFT_SERVER_LOAD_ENABLED, value);
  }

  static bool getThriftServerLoadEnabled() {
    return folly::fibers::local<McrouterFiberContext>().featureFlags.test(
        FeatureFlag::THRIFT_SERVER_LOAD_ENABLED);
  }

  /**
   * Add callback function to compute extra data for logging.
   * @return The index of new added callback function.
   */
  static size_t addExtraDataCallbacks(ExtraDataCallbackT&& callback) {
    auto& callbacks =
        folly::fibers::local<McrouterFiberContext>().extraDataCallbacks;
    callbacks.push_back(std::move(callback));
    return callbacks.size() - 1;
  }

  /**
   * Update callback function to compute extra data for logging on position idx.
   */
  static void updateExtraDataCallbacks(
      size_t idx,
      ExtraDataCallbackT&& callback) {
    folly::fibers::local<McrouterFiberContext>().extraDataCallbacks[idx] =
        std::move(callback);
  }

  /**
   * Return all callback functions to compute extra data for logging.
   */
  static const std::vector<ExtraDataCallbackT>& getExtraDataCallbacks() {
    return folly::fibers::local<McrouterFiberContext>().extraDataCallbacks;
  }

  /**
   * Update AxonContext for current fiber (thread, if we're not on
   * fiber)
   */
  static void setAxonCtx(std::shared_ptr<AxonContext> ctx) {
    folly::fibers::local<McrouterFiberContext>().axonCtx = std::move(ctx);
  }

  /**
   * Get AxonContext of current fiber (thread, if we're not on fiber)
   */
  static std::shared_ptr<AxonContext>& getAxonCtx() {
    return folly::fibers::local<McrouterFiberContext>().axonCtx;
  }

  /**
   * When bucketized routing is enabled, McBucketRoute will propagate
   * the calculated bucket id down the routing tree via this context.
   */
  static void setBucketId(uint64_t bucketId) {
    folly::fibers::local<McrouterFiberContext>().bucketId = bucketId;
  }

  static std::optional<uint64_t> getBucketId() {
    return folly::fibers::local<McrouterFiberContext>().bucketId;
  }

  static void setDistributionTargetRegion(std::string region) {
    folly::fibers::local<McrouterFiberContext>().distributionTargetRegion =
        region;
  }

  static std::optional<std::string> getDistributionTargetRegion() {
    return folly::fibers::local<McrouterFiberContext>()
        .distributionTargetRegion;
  }
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
