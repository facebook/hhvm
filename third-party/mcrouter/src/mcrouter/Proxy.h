/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <dirent.h>
#include <event.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/types.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <folly/Range.h>
#include <folly/SharedMutex.h>
#include <folly/Synchronized.h>
#include <folly/concurrency/CacheLocality.h>

#include "mcrouter/ExponentialSmoothData.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyRequestPriority.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/carbon/Keys.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/mc/protocol.h"
#include "mcrouter/lib/network/AsyncMcClient.h"
#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/ThriftTransport.h"
#include "mcrouter/lib/network/UniqueIntrusiveList.h"
#include "mcrouter/options.h"

namespace folly {
struct dynamic;
class File;
} // namespace folly

namespace facebook {
namespace memcache {

template <class T>
class MessageQueue;

namespace mcrouter {
// forward declaration
template <class RouterInfo>
class CarbonRouterClient;
template <class RouterInfo>
class CarbonRouterInstance;
class CarbonRouterInstanceBase;
template <class RouterInfo>
class ProxyConfig;
class ProxyRequestContext;
template <class RouterInfo, class Request>
class ProxyRequestContextTyped;
class ShardSplitter;

struct ProxyMessage {
  enum class Type { REQUEST, OLD_CONFIG, REPLACE_AP, SHUTDOWN };

  Type type{Type::REQUEST};
  void* data{nullptr};

  constexpr ProxyMessage() = default;

  ProxyMessage(Type t, void* d) noexcept : type(t), data(d) {}
};

// struct used for replace message
//
struct replace_ap_t {
  explicit replace_ap_t(
      const AccessPoint& oldAp,
      std::shared_ptr<const AccessPoint> newAp)
      : oldAccessPoint(oldAp), newAccessPoint(std::move(newAp)) {}

  // oldAccessPoint contains IP+port+other AP related info of the host that just
  //                got removed
  AccessPoint oldAccessPoint;
  // newAccessPoint contains IP+port+other AP related info of the host that just
  //                got added
  std::shared_ptr<const AccessPoint> newAccessPoint;
  // replacedAccessPoint
  //                is the RETURN value from replace command which contains the
  //                actual access point corresponding to the removed host
  //                which is in the internal data structures like
  //                ProxyDestination, ProxyDestinationMap, AccessPoints set in
  //                ProxyConfig.
  //                Since this is the actual AP present in the internal data
  //                structures, we need to find this using the oldAccessPoint
  //                and querying ProxyDestinationMap
  std::shared_ptr<const AccessPoint> replacedAccessPoint;

  // baton to wait on for the completion of the message processing
  folly::fibers::Baton baton;
};

template <class RouterInfo>
class Proxy : public ProxyBase {
 public:
  ~Proxy() override;

  /**
   * Access to config - can only be called on the proxy thread
   * and the resulting shared_ptr can only be detroyed on the proxy thread.
   */
  std::shared_ptr<ProxyConfig<RouterInfo>> getConfigUnsafe() const;

  /**
   * Can be called from any thread.
   *
   * Returns a lock and a reference to the config.
   * The caller may only access the config through the reference
   * while the lock is held.
   */
  std::pair<std::shared_lock<folly::SharedMutex>, ProxyConfig<RouterInfo>&>
  getConfigLocked() const;

  /**
   * Thread-safe config swap; returns the previous contents of
   * the config pointer
   */
  std::shared_ptr<ProxyConfig<RouterInfo>> swapConfig(
      std::shared_ptr<ProxyConfig<RouterInfo>> newConfig);

  /** Queue up and route the new incoming request */
  template <class Request>
  void dispatchRequest(
      const Request& req,
      std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>> ctx);

  /**
   * Put a new proxy message into the queue.
   */
  void sendMessage(ProxyMessage::Type t, void* data) noexcept;

  /**
   * @return Current value of the relaxed notification period if set.
   */
  size_t queueNotifyPeriod() const override;

  bool beingDestroyed() const {
    return beingDestroyed_;
  }

  typename RouterInfo::RouterStats& requestStats() {
    return requestStats_;
  }

  folly::dynamic dumpRequestStats(bool filterZeroes) const final {
    return requestStats_.dump(filterZeroes);
  }

  void processReplaceMessage(replace_ap_t* rep);

  // replaceAP
  // @oldAP    temporary AP constructed from IP address of the host
  //           that got replaced
  // @newAP    new AP that is used to replace oldAP in the ProxyDestination and
  //           ProxyDestinationMap
  //
  // Returns
  // replacedAP    AP in ProxyDestinationMap corresponding to the host that got
  //           replaced, this is the real AP to be replaced. oldAP is used
  //           to find this real AP
  // Replace message is sent to proxy thread to find the AP to be replaced
  // using 'oldAP' and then the AP is replaced by newAP and the replaced AP
  // is returned to caller
  std::shared_ptr<const AccessPoint> replaceAP(
      const AccessPoint& oldAP,
      std::shared_ptr<const AccessPoint> newAP);

  void advanceRequestStatsBin() override {
    requestStats().advanceBin();
  }

  bool messageQueueFull() const noexcept override;

 private:
  // If true, processing new requests is not safe.
  bool beingDestroyed_{false};

  /** Read/write lock for config pointer */
  mutable folly::SharedMutex configLock_;
  std::shared_ptr<ProxyConfig<RouterInfo>> config_;

  typename RouterInfo::RouterStats requestStats_;

  std::unique_ptr<MessageQueue<ProxyMessage>> messageQueue_;

  static Proxy<RouterInfo>* createProxy(
      CarbonRouterInstanceBase& router,
      folly::VirtualEventBase& evb,
      size_t id);
  Proxy(
      CarbonRouterInstanceBase& router,
      size_t id,
      folly::VirtualEventBase& evb);

  void messageReady(ProxyMessage::Type t, void* data);

  // Add task to route request through route handle tree
  template <class Request>
  typename std::enable_if_t<
      ListContains<typename RouterInfo::RoutableRequests, Request>::value>
  addRouteTask(
      const Request& req,
      std::shared_ptr<ProxyRequestContextTyped<RouterInfo, Request>> sharedCtx);
  // Fail all unknown operations
  template <class Request>
  typename std::enable_if_t<
      !ListContains<typename RouterInfo::RoutableRequests, Request>::value>
  addRouteTask(
      const Request& req,
      std::shared_ptr<ProxyRequestContextTyped<RouterInfo, Request>> sharedCtx);

  // Process and reply stats request
  void routeHandlesProcessRequest(
      const McStatsRequest& req,
      std::unique_ptr<ProxyRequestContextTyped<RouterInfo, McStatsRequest>>
          ctx);
  // Process and reply to a version request
  void routeHandlesProcessRequest(
      const McVersionRequest& req,
      std::unique_ptr<ProxyRequestContextTyped<RouterInfo, McVersionRequest>>
          ctx);
  // Process and reply to a get request
  void routeHandlesProcessRequest(
      const McGetRequest& req,
      std::unique_ptr<ProxyRequestContextTyped<RouterInfo, McGetRequest>> ctx);
  // Route request through route handle tree
  template <class Request>
  void routeHandlesProcessRequest(
      const Request& req,
      std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>> ctx);

  // Process request (update stats and route the request)
  template <class Request>
  void processRequest(
      const Request& req,
      std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>> ctx);

  /**
   * We use this wrapper instead of putting 'hook' inside ProxyRequestContext
   * directly due to an include cycle:
   * proxy.h -> ProxyRequestContext.h -> ProxyRequestLogger.h ->
   * ProxyRequestLogger-inl.h -> proxy.h
   */
  class WaitingRequestBase {
    UniqueIntrusiveListHook hook;

   public:
    using Queue =
        UniqueIntrusiveList<WaitingRequestBase, &WaitingRequestBase::hook>;

    virtual ~WaitingRequestBase() = default;

    /**
     * Continue processing proxy request.
     *
     * We lose any information about the type when we enqueue request as
     * waiting. The inheritance allows us to resume where we left and continues
     * processing requests retaining all the type information
     * (e.g. Operation and Request).
     */
    virtual void process(Proxy* proxy) = 0;
  };

  template <class Request>
  class WaitingRequest : public WaitingRequestBase {
   public:
    WaitingRequest(
        const Request& req,
        std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>> ctx);
    void process(Proxy* proxy) final;
    void setTimePushedOnQueue(int64_t now) {
      timePushedOnQueue_ = now;
    }

   private:
    const Request& req_;
    std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>> ctx_;

    int64_t timePushedOnQueue_{-1};
  };

  /** Queue of requests we didn't start processing yet */
  typename WaitingRequestBase::Queue
      waitingRequests_[static_cast<int>(ProxyRequestPriority::kNumPriorities)];

  /** If true, we can't start processing this request right now */
  template <class Request>
  typename std::enable_if<TNotRateLimited<Request>::value, bool>::type
  rateLimited(ProxyRequestPriority priority, const Request&) const;

  template <class Request>
  typename std::enable_if<!TNotRateLimited<Request>::value, bool>::type
  rateLimited(ProxyRequestPriority priority, const Request&) const;

  /** Will let through requests from the above queue if we have capacity */
  void pump() final;

  friend class CarbonRouterInstance<RouterInfo>;
  friend class CarbonRouterClient<RouterInfo>;
  friend class ProxyRequestContext;
};

template <class RouterInfo>
struct old_config_req_t {
  explicit old_config_req_t(std::shared_ptr<ProxyConfig<RouterInfo>> config)
      : config_(std::move(config)) {}

 private:
  std::shared_ptr<ProxyConfig<RouterInfo>> config_;
};

template <class RouterInfo>
void proxy_config_swap(
    Proxy<RouterInfo>* proxy,
    std::shared_ptr<ProxyConfig<RouterInfo>> config);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "Proxy-inl.h"
