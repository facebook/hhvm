/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cassert>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/ForEachPossibleClient.h"
#include "mcrouter/HostWithShard-fwd.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyRequestContextTyped.h"
#include "mcrouter/lib/RouteHandleTraverser.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {
template <class Request>
void bumpCarbonRouterClientStats(
    CacheClientStats& stats,
    const Request& req,
    const ReplyT<Request>& reply,
    carbon::GetLikeT<Request> = 0) {
  auto replyBytes = carbon::valuePtrUnsafe(reply)
      ? carbon::valuePtrUnsafe(reply)->computeChainDataLength()
      : 0;
  stats.recordFetchRequest(req.key_ref()->fullKey().size(), replyBytes);
}

template <class Request>
void bumpCarbonRouterClientStats(
    CacheClientStats& stats,
    const Request& req,
    const ReplyT<Request>&,
    carbon::UpdateLikeT<Request> = 0) {
  auto valueBytes = carbon::valuePtrUnsafe(req)
      ? carbon::valuePtrUnsafe(req)->computeChainDataLength()
      : 0;
  stats.recordUpdateRequest(req.key_ref()->fullKey().size(), valueBytes);
}

template <class Request>
void bumpCarbonRouterClientStats(
    CacheClientStats& stats,
    const Request& req,
    const ReplyT<Request>&,
    carbon::ArithmeticLikeT<Request> = 0) {
  stats.recordUpdateRequest(req.key_ref()->fullKey().size(), 0);
}

template <class Request>
void bumpCarbonRouterClientStats(
    CacheClientStats& stats,
    const Request& req,
    const ReplyT<Request>&,
    carbon::DeleteLikeT<Request> = 0) {
  stats.recordInvalidateRequest(req.key_ref()->fullKey().size());
}

template <class Request>
void bumpCarbonRouterClientStats(
    CacheClientStats&,
    const Request&,
    const ReplyT<Request>&,
    carbon::OtherThanT<
        Request,
        carbon::GetLike<>,
        carbon::UpdateLike<>,
        carbon::ArithmeticLike<>,
        carbon::DeleteLike<>> = 0) {
  // We don't have any other operation specific stats.
}

template <class Request>
const Request& unwrapRequest(const Request& req) {
  return req;
}

template <class Request>
const Request& unwrapRequest(std::reference_wrapper<const Request>& req) {
  return req.get();
}

bool srHostWithShardFuncCarbonRouterClient(
    const HostWithShard& hostWithShard,
    const RequestClass& requestClass,
    uint64_t& hash,
    uint64_t& hint);

} // namespace detail

template <class RouterInfo>
template <class Request, class F>
bool CarbonRouterClient<RouterInfo>::send(
    const Request& req,
    F&& callback,
    folly::StringPiece ipAddr) {
  auto makePreq = [this, ipAddr, &req, &callback](bool inBatch) mutable {
    return makeProxyRequestContext(
        req, std::forward<F>(callback), ipAddr, inBatch);
  };

  auto cancelRemaining = [&req, &callback]() {
    callback(req, ReplyT<Request>(carbon::Result::LOCAL_ERROR));
  };

  return sendMultiImpl(1, std::move(makePreq), std::move(cancelRemaining));
}

template <class RouterInfo>
template <class F, class G>
bool CarbonRouterClient<RouterInfo>::sendMultiImpl(
    size_t nreqs,
    F&& makeNextPreq,
    G&& failRemaining) {
  auto router = router_.lock();
  if (FOLLY_UNLIKELY(!router)) {
    return false;
  }

  auto notify = [this]() {
    assert(mode_ != ThreadMode::SameThread);
    if (mode_ == ThreadMode::FixedRemoteThread) {
      proxies_[proxyIdx_]->messageQueue_->notifyRelaxed();
    } else {
      assert(mode_ == ThreadMode::AffinitizedRemoteThread);
      size_t i = 0;
      for (const auto& p : proxies_) {
        if (proxiesToNotify_[i]) {
          p->messageQueue_->notifyRelaxed();
          proxiesToNotify_[i] = false;
        }
        ++i;
      }
    }
  };

  if (maxOutstanding() == 0) {
    if (mode_ == ThreadMode::SameThread) {
      for (size_t i = 0; i < nreqs; ++i) {
        sendSameThread(makeNextPreq(/* inBatch */ false));
      }
    } else {
      bool delayNotification = shouldDelayNotification(nreqs);
      for (size_t i = 0; i < nreqs; ++i) {
        sendRemoteThread(makeNextPreq(delayNotification), delayNotification);
      }
      if (delayNotification) {
        notify();
      }
    }
  } else if (maxOutstandingError()) {
    for (size_t begin = 0; begin < nreqs;) {
      auto end = begin +
          counting_sem_lazy_nonblocking(outstandingReqsSem(), nreqs - begin);
      if (begin == end) {
        failRemaining();
        break;
      }

      if (mode_ == ThreadMode::SameThread) {
        for (size_t i = begin; i < end; i++) {
          sendSameThread(makeNextPreq(/*  inBatch */ false));
        }
      } else {
        bool delayNotification = shouldDelayNotification(end - begin);
        for (size_t i = begin; i < end; i++) {
          sendRemoteThread(makeNextPreq(delayNotification), delayNotification);
        }
        if (delayNotification) {
          notify();
        }
      }

      begin = end;
    }
  } else {
    assert(mode_ != ThreadMode::SameThread);

    size_t i = 0;
    size_t n = 0;

    while (i < nreqs) {
      n += counting_sem_lazy_wait(outstandingReqsSem(), nreqs - n);
      bool delayNotification = shouldDelayNotification(n);
      for (size_t j = i; j < n; ++j) {
        sendRemoteThread(makeNextPreq(delayNotification), delayNotification);
      }
      if (delayNotification) {
        notify();
      }
      i = n;
    }
  }

  return true;
}

template <class RouterInfo>
template <class Request>
typename std::enable_if<
    ListContains<typename RouterInfo::RoutableRequests, Request>::value,
    std::pair<uint64_t, uint64_t>>::type
CarbonRouterClient<RouterInfo>::findAffinitizedProxyIdx(
    const Request& req) const {
  assert(mode_ == ThreadMode::AffinitizedRemoteThread);

  // Create a traverser
  uint64_t hash = 0;
  uint64_t hint = 0;
  RouteHandleTraverser<typename RouterInfo::RouteHandleIf> t(
      /* start */ nullptr,
      /* end */ nullptr,
      [&hash](const AccessPoint& ap, const PoolContext& poolContext) mutable {
        if (!poolContext.isShadow) {
          hash = ap.getHash();
          // if it's not a shadow and got the hash, we should stop the traversal
          return true;
        }
        return false;
      },
      [&hash, &hint](
          const HostWithShard& hostWithShard,
          const RequestClass& requestClass) {
        return detail::srHostWithShardFuncCarbonRouterClient(
            hostWithShard, requestClass, hash, hint);
      },
      [&hash](const AccessPoint& srHost, const RequestClass& requestClass) {
        if (!requestClass.is(RequestClass::kShadow) &&
            srHost.getTaskId().has_value()) {
          hash = srHost.getTaskId().value();
          return true;
        }
        return false;
      });

  // Traverse the routing tree.
  {
    auto config = proxies_[proxyIdx_]->getConfigLocked();
    config.second.proxyRoute().traverse(req, t);
  }

  return {hash % proxies_.size(), hint};
}

template <class RouterInfo>
template <class Request>
typename std::enable_if<
    !ListContains<typename RouterInfo::RoutableRequests, Request>::value,
    std::pair<uint64_t, uint64_t>>::type
CarbonRouterClient<RouterInfo>::findAffinitizedProxyIdx(
    const Request& /* unused */) const {
  assert(mode_ == ThreadMode::AffinitizedRemoteThread);
  return {0, 0};
}

template <class RouterInfo>
template <class InputIt, class F>
bool CarbonRouterClient<RouterInfo>::send(
    InputIt begin,
    InputIt end,
    F&& callback,
    folly::StringPiece ipAddr) {
  using IterReference = typename std::iterator_traits<InputIt>::reference;
  using Request = typename std::decay<decltype(detail::unwrapRequest(
      std::declval<IterReference>()))>::type;

  auto makeNextPreq = [this, ipAddr, &callback, &begin](bool inBatch) {
    auto proxyRequestContext = makeProxyRequestContext(
        detail::unwrapRequest(*begin), callback, ipAddr, inBatch);
    ++begin;
    return proxyRequestContext;
  };

  auto cancelRemaining = [&begin, &end, &callback]() {
    while (begin != end) {
      callback(
          detail::unwrapRequest(*begin),
          ReplyT<Request>(carbon::Result::LOCAL_ERROR));
      ++begin;
    }
  };

  return sendMultiImpl(
      std::distance(begin, end),
      std::move(makeNextPreq),
      std::move(cancelRemaining));
}

template <class RouterInfo>
void CarbonRouterClient<RouterInfo>::sendRemoteThread(
    std::unique_ptr<ProxyRequestContextWithInfo<RouterInfo>> req,
    bool skipNotification) {
  // Use the proxy saved in the ProxyRequestContext, as it may change
  // base on the ThreadMode.
  // Get a reference to the Proxy first as the unique pointer is released as
  // part of the blockingWriteRelaxed call.
  Proxy<RouterInfo>& pr = req->proxyWithRouterInfo();
  pr.messageQueue_->blockingWriteNoNotify(
      ProxyMessage::Type::REQUEST, req.release());
  if (!skipNotification) {
    pr.messageQueue_->notifyRelaxed();
  }
}

template <class RouterInfo>
void CarbonRouterClient<RouterInfo>::sendSameThread(
    std::unique_ptr<ProxyRequestContextWithInfo<RouterInfo>> req) {
  // We are guaranteed to be in the thread that owns proxies_[proxyIdx_]
  proxies_[proxyIdx_]->messageReady(ProxyMessage::Type::REQUEST, req.release());
}

template <class RouterInfo>
CarbonRouterClient<RouterInfo>::CarbonRouterClient(
    std::shared_ptr<CarbonRouterInstance<RouterInfo>> router,
    size_t maximumOutstanding,
    bool maximumOutstandingError,
    ThreadMode mode)
    : CarbonRouterClientBase(maximumOutstanding, maximumOutstandingError),
      router_(router),
      mode_(mode),
      proxies_(router->getProxies()),
      proxiesToNotify_(proxies_.size(), false) {
  // If the mode is SameThread, make sure to match the current EventBase with
  // the corresponding Proxy EventBase. This has the requirement that create
  // is called from an EventBase that's currently a Proxy EventBase.
  if (mode == ThreadMode::SameThread) {
    const auto& proxies = router->getProxies();
    auto evb = folly::EventBaseManager::get()->getExistingEventBase();
    for (size_t i = 0; i < proxies.size(); i++) {
      if (evb == &(proxies[i]->eventBase().getEventBase())) {
        proxyIdx_ = i;
        return;
      }
    }
    LOG(ERROR)
        << "Could not find a matching same-thread EVB when creating CarbonRouterClient";
    // We fall back to setting the proxy index with the next index if we
    // couldn't find a match.
  }
  proxyIdx_ = router->nextProxyIndex();
}

template <class RouterInfo>
typename CarbonRouterClient<RouterInfo>::Pointer
CarbonRouterClient<RouterInfo>::create(
    std::shared_ptr<CarbonRouterInstance<RouterInfo>> router,
    size_t maximumOutstanding,
    bool maximumOutstandingError,
    ThreadMode mode) {
  auto client = new CarbonRouterClient<RouterInfo>(
      std::move(router), maximumOutstanding, maximumOutstandingError, mode);
  client->self_ = std::shared_ptr<CarbonRouterClient>(client);
  return Pointer(client);
}

template <class RouterInfo>
CarbonRouterClient<RouterInfo>::~CarbonRouterClient() {
  assert(disconnected_);
}

template <class RouterInfo>
bool CarbonRouterClient<RouterInfo>::shouldDelayNotification(
    size_t batchSize) const {
  return mode_ != ThreadMode::SameThread && batchSize > 1;
}

template <class RouterInfo>
template <class Request, class CallbackFunc>
std::unique_ptr<ProxyRequestContextWithInfo<RouterInfo>>
CarbonRouterClient<RouterInfo>::makeProxyRequestContext(
    const Request& req,
    CallbackFunc&& callback,
    folly::StringPiece ipAddr,
    bool inBatch) {
  Proxy<RouterInfo>* proxy = proxies_[proxyIdx_];
  uint64_t routingHint = 0;
  if (mode_ == ThreadMode::AffinitizedRemoteThread) {
    auto [idx, hint] = findAffinitizedProxyIdx(req);
    routingHint = hint;
    proxy = proxies_[idx];
    if (inBatch) {
      proxiesToNotify_[idx] = true;
    }
  }
  auto proxyRequestContext = createProxyRequestContext(
      *proxy,
      req,
      [this, cb = std::forward<CallbackFunc>(callback)](
          auto& /* reqCtx */,
          const Request& request,
          ReplyT<Request>&& reply) mutable {
        detail::bumpCarbonRouterClientStats(stats_, request, reply);
        if (disconnected_) {
          // "Cancelled" reply.
          cb(request, ReplyT<Request>(carbon::Result::UNKNOWN));
        } else {
          cb(request, std::move(reply));
        }
      });

  if (routingHint) {
    proxyRequestContext->setRoutingHint(routingHint);
  }

  proxyRequestContext->setRequester(self_);
  if (!ipAddr.empty()) {
    proxyRequestContext->setUserIpAddress(ipAddr);
  }
  return proxyRequestContext;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
