/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Utility.h>
#include <string_view>

#include "mcrouter/ProxyRequestContext.h"
#include "mcrouter/ProxyRequestLogger.h"
#include "mcrouter/lib/RequestLoggerContext.h"
#include "mcrouter/lib/carbon/NoopAdditionalLogger.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
class ProxyConfig;
template <class RouterInfo>
class Proxy;
namespace detail {

template <class T>
struct Void {
  using type = void;
};

template <class RouterInfo, class U = void>
struct RouterAdditionalLogger {
  using type = carbon::NoopAdditionalLogger;
};
template <class RouterInfo>
struct RouterAdditionalLogger<
    RouterInfo,
    typename Void<typename RouterInfo::AdditionalLogger>::type> {
  using type = typename RouterInfo::AdditionalLogger;
};
} // namespace detail

template <class Logger, class Request, typename = std::enable_if_t<true>>
class HasLogBeforeRequestSent : public std::false_type {};

template <class Logger, class Request>
class HasLogBeforeRequestSent<
    Logger,
    Request,
    std::void_t<
        decltype(std::declval<std::decay_t<Logger>&>().logBeforeRequestSent(
            std::declval<Request>(),
            std::declval<RequestLoggerContext>()))>> : public std::true_type {};

template <class RouterInfo>
class ProxyRequestContextWithInfo : public ProxyRequestContext {
 public:
  /**
   * A request with this context will not be sent/logged anywhere.
   *
   * @param clientCallback  If non-nullptr, called by DestinationRoute when
   *   the request would normally be sent to destination;
   *   also in traverse() of DestinationRoute.
   * @param shardSplitCallback  If non-nullptr, called by ShardSplitRoute
   *   in traverse() with itself as the argument.
   */
  static std::shared_ptr<ProxyRequestContextWithInfo<RouterInfo>>
  createRecording(
      Proxy<RouterInfo>& proxy,
      ClientCallback clientCallback,
      ShardSplitCallback shardSplitCallback = nullptr,
      BucketizationCallback bucketizationCallback = nullptr) {
    return std::shared_ptr<ProxyRequestContextWithInfo<RouterInfo>>(
        new ProxyRequestContextWithInfo<RouterInfo>(
            Recording,
            proxy,
            std::move(clientCallback),
            std::move(shardSplitCallback),
            std::move(bucketizationCallback)));
  }

  /**
   * Same as createRecording(), but also notifies the baton
   * when this context is destroyed (i.e. all requests referencing it
   * finish executing).
   */
  static std::shared_ptr<ProxyRequestContextWithInfo<RouterInfo>>
  createRecordingNotify(
      Proxy<RouterInfo>& proxy,
      folly::fibers::Baton& baton,
      ClientCallback clientCallback,
      ShardSplitCallback shardSplitCallback = nullptr,
      BucketizationCallback bucketizationCallback = nullptr) {
    return std::shared_ptr<ProxyRequestContextWithInfo<RouterInfo>>(
        new ProxyRequestContextWithInfo<RouterInfo>(
            Recording,
            proxy,
            std::move(clientCallback),
            std::move(shardSplitCallback),
            std::move(bucketizationCallback)),
        [&baton](ProxyRequestContext* ctx) {
          delete ctx;
          baton.post();
        });
  }

  ~ProxyRequestContextWithInfo() override {
    if (auto poolStats = proxy_.stats().getPoolStats(poolStatIndex_)) {
      poolStats->incrementFinalResultErrorCount(
          isErrorResult(finalResult_) ? 1 : 0);
      poolStats->addTotalDurationSample(nowUs() - startDurationUs_);
    }
  }

  /**
   * Called before a request is sent
   */
  template <class Request>
  void onBeforeRequestSent(
      const folly::StringPiece poolName,
      const AccessPoint& ap,
      folly::StringPiece strippedRoutingPrefix,
      const Request& request,
      RequestClass requestClass,
      const int64_t startTimeUs,
      const std::string_view bucketId = std::string_view(),
      const RequestLoggerContextFlags flags = RequestLoggerContextFlags::NONE) {
    if constexpr (HasLogBeforeRequestSent<AdditionalLogger, Request>::value) {
      if (recording()) {
        return;
      }

      RpcStatsContext rpcStatsContext;
      RequestLoggerContext loggerContext(
          poolName,
          ap,
          strippedRoutingPrefix,
          requestClass,
          startTimeUs,
          /* endTimeUs */ 0,
          carbon::Result::UNKNOWN,
          rpcStatsContext,
          /* networkTransportTimeUs */ 0,
          /* extraDataCallbacks */ {},
          bucketId,
          flags,
          /* numFailovers */ 0,
          /* beforeReqLatencyInjectedUs */ 0,
          /* afterReqLatencyInjectedUs */ 0,
          /* poolIndex */ std::nullopt,
          getProductId(request));

      assert(additionalLogger_.hasValue());
      additionalLogger_->logBeforeRequestSent(request, loggerContext);
    }
  }

  /**
   * Called once a reply is received to record a stats sample if required.
   */
  template <class Request>
  void onReplyReceived(
      const folly::StringPiece poolName,
      std::optional<size_t> poolIndex,
      const AccessPoint& ap,
      folly::StringPiece strippedRoutingPrefix,
      const Request& request,
      const ReplyT<Request>& reply,
      RequestClass requestClass,
      const int64_t startTimeUs,
      const int64_t endTimeUs,
      const int32_t poolStatIndex,
      const RpcStatsContext rpcStatsContext,
      const int64_t networkTransportTimeUs,
      const std::vector<ExtraDataCallbackT>& extraDataCallbacks,
      const std::string_view bucketId = std::string_view(),
      const RequestLoggerContextFlags flags = RequestLoggerContextFlags::NONE) {
    if (recording()) {
      return;
    }

    if (auto poolStats = proxy_.stats().getPoolStats(poolStatIndex)) {
      poolStats->incrementRequestCount(1);
      poolStats->addDurationSample(endTimeUs - startTimeUs);
    }
    RequestLoggerContext loggerContext(
        poolName,
        ap,
        strippedRoutingPrefix,
        requestClass,
        startTimeUs,
        endTimeUs,
        *reply.result_ref(),
        rpcStatsContext,
        networkTransportTimeUs,
        extraDataCallbacks,
        bucketId,
        flags,
        fiber_local<RouterInfo>::getFailoverCount(),
        fiber_local<RouterInfo>::getAccumulatedInjectedBeforeReqLatencyUs(),
        fiber_local<RouterInfo>::getAccumulatedInjectedAfterReqLatencyUs(),
        poolIndex,
        getProductId(request));
    assert(logger_.hasValue());
    logger_->template log<Request>(loggerContext);
    assert(additionalLogger_.hasValue());
    additionalLogger_->log(request, reply, loggerContext);
  }

  template <class KeyType>
  bool mayLog(
      const carbon::Keys<KeyType>& key,
      const RequestClass& reqClass,
      const carbon::Result& replyResult,
      const int64_t durationUs) const {
    return additionalLogger_->mayLog(key, reqClass, replyResult, durationUs);
  }

 public:
  Proxy<RouterInfo>& proxyWithRouterInfo() const {
    return proxy_;
  }

  using AdditionalLogger =
      typename detail::RouterAdditionalLogger<RouterInfo>::type;
  AdditionalLogger& additionalLogger() {
    return *additionalLogger_;
  }

 protected:
  ProxyRequestContextWithInfo(
      Proxy<RouterInfo>& pr,
      ProxyRequestPriority priority__,
      const void* ptr = nullptr)
      : ProxyRequestContext(pr, priority__, ptr),
        proxy_(pr),
        logger_(folly::in_place, pr),
        additionalLogger_(folly::in_place, *this) {}

  Proxy<RouterInfo>& proxy_;

 private:
  ProxyRequestContextWithInfo(
      RecordingT,
      Proxy<RouterInfo>& pr,
      ClientCallback clientCallback,
      ShardSplitCallback shardSplitCallback = nullptr,
      BucketizationCallback bucketizationCallback = nullptr)
      : ProxyRequestContext(
            Recording,
            pr,
            std::move(clientCallback),
            std::move(shardSplitCallback),
            std::move(bucketizationCallback)),
        proxy_(pr) {}

  int64_t startDurationUs_{nowUs()};
  folly::Optional<ProxyRequestLogger<RouterInfo>> logger_;
  folly::Optional<AdditionalLogger> additionalLogger_;
};

template <class RouterInfo, class Request>
class ProxyRequestContextTyped
    : public ProxyRequestContextWithInfo<RouterInfo> {
 public:
  using Type = ProxyRequestContextTyped<RouterInfo, Request>;
  /**
   * Sends the reply for this proxy request.
   * @param newReply the message that we are sending out as the reply
   *   for the request we are currently handling
   */
  void sendReply(ReplyT<Request>&& reply);

  /**
   * DEPRECATED. Convenience method, that constructs reply and calls
   * non-template method.
   *
   * WARNING: This function can be dangerous with new typed requests.
   * For typed requests,
   *   ctx->sendReply(carbon::Result::LOCAL_ERROR, "Error message")
   * does the right thing, while
   *   ctx->sendReply(carbon::Result::FOUND, "value")
   * does the wrong thing.
   */
  template <class... Args>
  void sendReply(Args&&... args) {
    sendReply(ReplyT<Request>(std::forward<Args>(args)...));
  }

  void startProcessing() final;

  const ProxyConfig<RouterInfo>& proxyConfig() const {
    assert(!this->recording());
    return *config_;
  }

  ProxyRoute<RouterInfo>& proxyRoute() const {
    assert(!this->recording());
    return config_->proxyRoute();
  }

  /**
   * Internally converts the context into one ready to route.
   * Config pointer is saved to keep the config alive, and
   * ownership is changed to shared so that all subrequests
   * keep track of this context.
   */
  static std::shared_ptr<Type> process(
      std::unique_ptr<Type> preq,
      std::shared_ptr<const ProxyConfig<RouterInfo>> config);

 protected:
  ProxyRequestContextTyped(
      Proxy<RouterInfo>& pr,
      const Request& req,
      ProxyRequestPriority priority__)
      : ProxyRequestContextWithInfo<RouterInfo>(
            pr,
            priority__,
            (const void*)&req) {}

  std::shared_ptr<const ProxyConfig<RouterInfo>> config_;

  // It's guaranteed to point to an existing request until we call user callback
  // (i.e. replied_ changes to true), after that it's nullptr.
  const Request* typedRequest() {
    return static_cast<const Request*>(this->ptr_);
  }

  void clearTypedRequest() {
    this->ptr_ = nullptr;
  }

  virtual void sendReplyImpl(ReplyT<Request>&& reply) = 0;

 private:
  friend class Proxy<RouterInfo>;
};

/**
 * Creates a new proxy request context
 */
template <class RouterInfo, class Request, class F>
std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>>
createProxyRequestContext(
    Proxy<RouterInfo>& pr,
    const Request& req,
    F&& f,
    ProxyRequestPriority priority = ProxyRequestPriority::kCritical);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "ProxyRequestContextTyped-inl.h"
