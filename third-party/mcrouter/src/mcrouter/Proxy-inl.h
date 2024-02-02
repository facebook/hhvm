/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Range.h>
#include <folly/fibers/EventBaseLoopController.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyRequestContextTyped.h"
#include "mcrouter/lib/MessageQueue.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/carbon/Stats.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/ProxyRoute.h"
#include "mcrouter/stats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
class ProxyConfig;

namespace detail {

#if defined(__GNUC__) && !defined(__clang__)
// Gcc wants the mutable keyword first
#define FOLLY_NOINLINE_MUTABLE mutable FOLLY_NOINLINE
#else
#define FOLLY_NOINLINE_MUTABLE FOLLY_NOINLINE mutable
#endif

template <class RouterInfo>
bool processGetServiceInfoRequest(
    const McGetRequest& req,
    std::shared_ptr<ProxyRequestContextTyped<RouterInfo, McGetRequest>>& ctx) {
  constexpr folly::StringPiece kInternalGetPrefix("__mcrouter__.");

  if (!req.key_ref()->fullKey().startsWith(kInternalGetPrefix)) {
    return false;
  }
  auto& config = ctx->proxyConfig();
  auto key = req.key_ref()->fullKey();
  key.advance(kInternalGetPrefix.size());
  config.serviceInfo()->handleRequest(key, ctx);
  return true;
}

} // namespace detail

template <class RouterInfo>
template <class Request>
Proxy<RouterInfo>::WaitingRequest<Request>::WaitingRequest(
    const Request& req,
    std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>> ctx)
    : req_(req), ctx_(std::move(ctx)) {}

template <class RouterInfo>
template <class Request>
void Proxy<RouterInfo>::WaitingRequest<Request>::process(
    Proxy<RouterInfo>* proxy) {
  // timePushedOnQueue_ is nonnegative only if waiting-requests-timeout is
  // enabled
  if (timePushedOnQueue_ >= 0) {
    const auto durationInQueueUs = nowUs() - timePushedOnQueue_;

    if (durationInQueueUs >
        1000 *
            static_cast<int64_t>(
                proxy->getRouterOptions().waiting_request_timeout_ms)) {
      ctx_->sendReply(carbon::Result::BUSY);
      return;
    }
  }

  proxy->processRequest(req_, std::move(ctx_));
}

template <class RouterInfo>
template <class Request>
typename std::enable_if_t<
    ListContains<typename RouterInfo::RoutableRequests, Request>::value>
Proxy<RouterInfo>::addRouteTask(
    const Request& req,
    std::shared_ptr<ProxyRequestContextTyped<RouterInfo, Request>> sharedCtx) {
  requestStats_.template bump<Request>(carbon::RouterStatTypes::Incoming);

  auto funcCtx = sharedCtx;

  fiberManager().addTaskFinally(
      [&req, ctx = std::move(funcCtx)]() FOLLY_NOINLINE_MUTABLE {
        try {
          auto& proute = ctx->proxyRoute();
          fiber_local<RouterInfo>::setSharedCtx(std::move(ctx));
          return proute.route(req);
        } catch (const std::exception& e) {
          auto err = folly::sformat(
              "Error routing request of type {}!"
              " Exception: {}",
              typeid(Request).name(),
              e.what());
          ReplyT<Request> reply(carbon::Result::LOCAL_ERROR);
          carbon::setMessageIfPresent(reply, std::move(err));
          return reply;
        }
      },
      [ctx = std::move(sharedCtx)](folly::Try<ReplyT<Request>>&& reply) {
        ctx->sendReply(std::move(*reply));
      });
}

template <class RouterInfo>
template <class Request>
typename std::enable_if_t<
    !ListContains<typename RouterInfo::RoutableRequests, Request>::value>
Proxy<RouterInfo>::addRouteTask(
    const Request&,
    std::shared_ptr<ProxyRequestContextTyped<RouterInfo, Request>> sharedCtx) {
  ReplyT<Request> reply(carbon::Result::LOCAL_ERROR);
  carbon::setMessageIfPresent(
      reply,
      folly::sformat(
          "Couldn't route request of type {} "
          "because the operation is not supported by RouteHandles "
          "library!",
          typeid(Request).name()));
  sharedCtx->sendReply(std::move(reply));
}

template <class RouterInfo>
template <class Request>
void Proxy<RouterInfo>::routeHandlesProcessRequest(
    const Request& req,
    std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>> uctx) {
  auto sharedCtx = ProxyRequestContextTyped<RouterInfo, Request>::process(
      std::move(uctx), getConfigUnsafe());

  addRouteTask(req, std::move(sharedCtx));
}

template <class RouterInfo>
template <class Request>
void Proxy<RouterInfo>::processRequest(
    const Request& req,
    std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>> ctx) {
  assert(!ctx->isProcessing());
  ctx->markAsProcessing();
  ++numRequestsProcessing_;
  stats().increment(proxy_reqs_processing_stat);
  if (FOLLY_UNLIKELY(req.getCryptoAuthToken().has_value())) {
    stats().increment(request_has_crypto_auth_token_stat);
  }
  ctx->runPreprocessFunction();
  routeHandlesProcessRequest(req, std::move(ctx));

  stats().increment(request_sent_stat);
  stats().increment(request_sent_count_stat);
}

template <class RouterInfo>
template <class Request>
void Proxy<RouterInfo>::dispatchRequest(
    const Request& req,
    std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>> ctx) {
  if (rateLimited(ctx->priority(), req)) {
    if (getRouterOptions().proxy_max_throttled_requests > 0 &&
        numRequestsWaiting_ >=
            getRouterOptions().proxy_max_throttled_requests) {
      ctx->sendReply(carbon::Result::BUSY);
      return;
    }
    auto& queue = waitingRequests_[static_cast<int>(ctx->priority())];
    auto w = std::make_unique<WaitingRequest<Request>>(req, std::move(ctx));
    // Only enable timeout on waitingRequests_ queue when queue throttling is
    // enabled
    if (getRouterOptions().proxy_max_inflight_requests > 0 &&
        getRouterOptions().proxy_max_throttled_requests > 0 &&
        getRouterOptions().waiting_request_timeout_ms > 0) {
      w->setTimePushedOnQueue(nowUs());
    }
    queue.pushBack(std::move(w));
    ++numRequestsWaiting_;
    stats().increment(proxy_reqs_waiting_stat);
  } else {
    processRequest(req, std::move(ctx));
  }
}

template <class RouterInfo>
Proxy<RouterInfo>::Proxy(
    CarbonRouterInstanceBase& rtr,
    size_t id,
    folly::VirtualEventBase& evb)
    : ProxyBase(rtr, id, evb, RouterInfo()) {
  messageQueue_ = std::make_unique<MessageQueue<ProxyMessage>>(
      router().opts().client_queue_size,
      [this](ProxyMessage&& message) {
        this->messageReady(message.type, message.data);
      },
      router().opts().client_queue_no_notify_rate,
      router().opts().client_queue_wait_threshold_us,
      &nowUs,
      [this]() { stats().incrementSafe(client_queue_notifications_stat); },
      [this, noFlushLoops = 0](bool last) mutable {
        bool haveTasks = fiberManager().runQueueSize() != 0;
        if (!last) {
          // If we have tasks in fiber manager, or we have pending flushes, then
          // we can guarantee that we won't block event loop.
          return haveTasks || !flushList().empty();
        }
        if (!flushList().empty() &&
            (!haveTasks ||
             ++noFlushLoops >= router().opts().max_no_flush_event_loops)) {
          noFlushLoops = 0;
          flushCallback_.setList(std::move(flushList()));
          eventBase().getEventBase().runInLoop(
              &flushCallback_, true /* thisIteration */);
        }
        return false;
      });
}

template <class RouterInfo>
Proxy<RouterInfo>* Proxy<RouterInfo>::createProxy(
    CarbonRouterInstanceBase& router,
    folly::VirtualEventBase& eventBase,
    size_t id) {
  auto proxy = std::unique_ptr<Proxy>(new Proxy(router, id, eventBase));
  auto proxyPtr = proxy.get();

  eventBase.runInEventBaseThread([proxyPtr, &eventBase]() {
    proxyPtr->messageQueue_->attachEventBase(eventBase);

    dynamic_cast<folly::fibers::EventBaseLoopController&>(
        proxyPtr->fiberManager().loopController())
        .attachEventBase(eventBase);

    std::chrono::milliseconds connectionResetInterval{
        proxyPtr->router().opts().reset_inactive_connection_interval};

    if (connectionResetInterval.count() > 0) {
      proxyPtr->destinationMap()->setResetTimer(connectionResetInterval);
    }
  });

  // We want proxy life-time to be tied to VirtualEventBase.
  eventBase.runOnDestruction([proxy = std::move(proxy)]() mutable {
    /* make sure proxy is deleted on the proxy thread */
    proxy.reset();
  });

  return proxyPtr;
}

template <class RouterInfo>
std::shared_ptr<ProxyConfig<RouterInfo>> Proxy<RouterInfo>::getConfigUnsafe()
    const {
  folly::SharedMutex::ReadHolder lg(configLock_);
  return config_;
}

template <class RouterInfo>
std::pair<std::shared_lock<folly::SharedMutex>, ProxyConfig<RouterInfo>&>
Proxy<RouterInfo>::getConfigLocked() const {
  auto lock = std::shared_lock(configLock_);
  /* make_pair strips the reference, so construct directly */
  return std::
      pair<std::shared_lock<folly::SharedMutex>, ProxyConfig<RouterInfo>&>(
          std::move(lock), *config_);
}

template <class RouterInfo>
std::shared_ptr<ProxyConfig<RouterInfo>> Proxy<RouterInfo>::swapConfig(
    std::shared_ptr<ProxyConfig<RouterInfo>> newConfig) {
  std::unique_lock lg(configLock_);
  auto old = std::move(config_);
  config_ = std::move(newConfig);
  return old;
}

/** drain and delete proxy object */
template <class RouterInfo>
Proxy<RouterInfo>::~Proxy() {
  destinationMap_.reset();

  beingDestroyed_ = true;

  if (messageQueue_) {
    messageQueue_->drain();
  }
}

template <class RouterInfo>
void Proxy<RouterInfo>::sendMessage(ProxyMessage::Type t, void* data) noexcept {
  CHECK(messageQueue_.get());
  messageQueue_->blockingWrite(t, data);
}

template <class RouterInfo>
size_t Proxy<RouterInfo>::queueNotifyPeriod() const {
  if (messageQueue_) {
    return messageQueue_->currentNotifyPeriod();
  }
  return 0;
}

template <class RouterInfo>
void Proxy<RouterInfo>::messageReady(ProxyMessage::Type t, void* data) {
  switch (t) {
    case ProxyMessage::Type::REQUEST: {
      auto preq = reinterpret_cast<ProxyRequestContext*>(data);
      preq->startProcessing();
    } break;

    case ProxyMessage::Type::OLD_CONFIG: {
      auto oldConfig = reinterpret_cast<old_config_req_t<RouterInfo>*>(data);
      delete oldConfig;
    } break;

    case ProxyMessage::Type::REPLACE_AP: {
      auto rep = reinterpret_cast<replace_ap_t*>(data);
      processReplaceMessage(rep);
      rep->baton.post();
    } break;

    case ProxyMessage::Type::SHUTDOWN:
      /*
       * No-op. We just wanted to wake this event base up so that
       * it can exit event loop and check router->shutdown
       */
      break;
  }
}

template <class RouterInfo>
void Proxy<RouterInfo>::routeHandlesProcessRequest(
    const McStatsRequest& req,
    std::unique_ptr<ProxyRequestContextTyped<RouterInfo, McStatsRequest>> ctx) {
  McStatsReply reply;
  try {
    reply = stats_reply(this, req.key_ref()->fullKey());
  } catch (const std::exception& e) {
    reply.result_ref() = carbon::Result::LOCAL_ERROR;
    reply.message_ref() =
        folly::to<std::string>("Error processing stats request: ", e.what());
  }
  ctx->sendReply(std::move(reply));
}

template <class RouterInfo>
void Proxy<RouterInfo>::routeHandlesProcessRequest(
    const McVersionRequest&,
    std::unique_ptr<ProxyRequestContextTyped<RouterInfo, McVersionRequest>>
        ctx) {
  McVersionReply reply(carbon::Result::OK);
  reply.value_ref() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, MCROUTER_PACKAGE_STRING);
  ctx->sendReply(std::move(reply));
}

template <class RouterInfo>
void Proxy<RouterInfo>::routeHandlesProcessRequest(
    const McGetRequest& req,
    std::unique_ptr<ProxyRequestContextTyped<RouterInfo, McGetRequest>> uctx) {
  auto sharedCtx = ProxyRequestContextTyped<RouterInfo, McGetRequest>::process(
      std::move(uctx), getConfigUnsafe());

  if (detail::processGetServiceInfoRequest(req, sharedCtx)) {
    return;
  }

  addRouteTask(req, std::move(sharedCtx));
}

template <class RouterInfo>
void Proxy<RouterInfo>::pump() {
  auto numPriorities = static_cast<int>(ProxyRequestPriority::kNumPriorities);
  for (int i = 0; i < numPriorities; ++i) {
    auto& queue = waitingRequests_[i];
    while (numRequestsProcessing_ <
               router().opts().proxy_max_inflight_requests &&
           !queue.empty()) {
      --numRequestsWaiting_;
      auto w = queue.popFront();
      stats().decrement(proxy_reqs_waiting_stat);

      w->process(this);
    }
  }
}

template <class RouterInfo>
void proxy_config_swap(
    Proxy<RouterInfo>* proxy,
    std::shared_ptr<ProxyConfig<RouterInfo>> config) {
  auto oldConfig = proxy->swapConfig(std::move(config));
  proxy->stats().setValue(config_last_success_stat, time(nullptr));

  if (oldConfig) {
    auto configReq = new old_config_req_t<RouterInfo>(std::move(oldConfig));
    proxy->sendMessage(ProxyMessage::Type::OLD_CONFIG, configReq);
  }
}

template <class RouterInfo>
template <class Request>
typename std::enable_if<TNotRateLimited<Request>::value, bool>::type
Proxy<RouterInfo>::rateLimited(ProxyRequestPriority, const Request&) const {
  return false;
}

template <class RouterInfo>
template <class Request>
typename std::enable_if<!TNotRateLimited<Request>::value, bool>::type
Proxy<RouterInfo>::rateLimited(ProxyRequestPriority priority, const Request&)
    const {
  if (!getRouterOptions().proxy_max_inflight_requests) {
    return false;
  }

  if (waitingRequests_[static_cast<int>(priority)].empty() &&
      numRequestsProcessing_ < getRouterOptions().proxy_max_inflight_requests) {
    return false;
  }

  return true;
}

template <class RouterInfo>
bool Proxy<RouterInfo>::messageQueueFull() const noexcept {
  return messageQueue_->isFull();
}

template <class RouterInfo>
void Proxy<RouterInfo>::processReplaceMessage(replace_ap_t* rep) {
  rep->replacedAccessPoint = nullptr;
  if (rep->oldAccessPoint.getProtocol() == mc_thrift_protocol) {
    rep->replacedAccessPoint =
        destinationMap_->replace<ThriftTransport<RouterInfo>>(
            rep->oldAccessPoint,
            rep->newAccessPoint,
            std::chrono::milliseconds(0));
  } else if (rep->oldAccessPoint.getProtocol() == mc_caret_protocol) {
    rep->replacedAccessPoint = destinationMap_->replace<AsyncMcClient>(
        rep->oldAccessPoint, rep->newAccessPoint, std::chrono::milliseconds(0));
  }
}

template <class RouterInfo>
std::shared_ptr<const AccessPoint> Proxy<RouterInfo>::replaceAP(
    const AccessPoint& oldAP,
    std::shared_ptr<const AccessPoint> newAP) {
  // Send message to replace AP in the ProxyDestination
  try {
    auto repMsg = std::make_unique<replace_ap_t>(oldAP, newAP);
    sendMessage(ProxyMessage::Type::REPLACE_AP, repMsg.get());
    // wait for the response
    repMsg->baton.wait();
    auto replacedAP = repMsg->replacedAccessPoint;
    VLOG(2) << ((replacedAP != nullptr) ? "SUCCESS " : "FAILED ")
            << " Replacing oldAP = " << oldAP.toString()
            << " newAP = " << newAP->toString();
    return replacedAP;
  } catch (...) {
    return nullptr;
  }
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
