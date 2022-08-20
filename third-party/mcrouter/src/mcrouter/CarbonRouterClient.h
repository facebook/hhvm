/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/IntrusiveList.h>
#include <folly/Range.h>

#include "mcrouter/CarbonRouterClientBase.h"
#include "mcrouter/lib/CacheClientStats.h"
#include "mcrouter/lib/fbi/cpp/TypeList.h"
#include "mcrouter/lib/mc/msg.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
class CarbonRouterClient;
template <class RouterInfo>
class CarbonRouterInstance;
template <class RouterInfo>
class Proxy;

class ProxyRequestContext;

template <class RouterInfo>
class ProxyRequestContextWithInfo;

/**
 * A mcrouter client is used to communicate with a mcrouter instance.
 * Typically a client is long lived. Request sent through a single client
 * will be sent to the same mcrouter thread that's determined once on creation.
 *
 * Create via CarbonRouterInstance::createClient().
 */
template <class RouterInfo>
class CarbonRouterClient : public CarbonRouterClientBase {
 private:
  struct Disconnecter {
    void operator()(CarbonRouterClient<RouterInfo>* client) {
      client->disconnected_ = true;
      /* We only access self_ when we need to send a request, which only
         the user can do. Since the user is destroying the pointer,
         there could be no concurrent send and this write is safe.

         Note: not client->self_.reset(), since this could destroy client
         from inside the call to reset(), destroying self_ while the method
         is still running. */
      auto stolenPtr = std::move(client->self_);
    }
  };
  folly::IntrusiveListHook hook_;

 public:
  using Pointer = std::unique_ptr<CarbonRouterClient<RouterInfo>, Disconnecter>;
  using Queue = folly::IntrusiveList<
      CarbonRouterClient<RouterInfo>,
      &CarbonRouterClient<RouterInfo>::hook_>;

  enum class ThreadMode {
    // Routes the request in the same thread that is calling CarbonRouterClient.
    SameThread = 0,

    // Routes the request in a dedicated mcrouter thread, chose at
    // CarbonRouterClient creation time.
    FixedRemoteThread,

    // Routes the request deterministically, chosen at routing time, with the
    // goal to reduce the number of connections between client and server.
    AffinitizedRemoteThread,
  };

  /**
   * Asynchronously send a single request with the given operation.
   *
   * @param req       The request to send.
   *                  The caller is responsible for keeping the request alive
   *                  until the callback is called.
   * @param callback  The callback to call when request is completed.
   *                  Should have the following signature:
   *                    f(const Request& request, ReplyT<Request>&& reply)
   *                  The reply.result_ref() carbon::Result::UNKNOWN means
   *                  that the request was canceled.
   *                  The callback will be moved into a temporary storage
   *                  before being called.
   *                  The callback will be destroyed only after callback is
   *                  called, but may be delayed, until all sub-requests are
   *                  processed.
   *                  The callback must be copyable.
   *
   * @return true iff the request was scheduled to be sent / was sent,
   *         false if some error happened (e.g. RouterInstance was destroyed).
   *
   * Note: the caller is responsible for keeping the request alive until the
   *       callback is called.
   */
  template <class Request, class F>
  bool send(
      const Request& req,
      F&& callback,
      folly::StringPiece ipAddr = folly::StringPiece());

  /**
   * Multi requests version of send.
   *
   * @param being     Iterator pointing to the first request.
   * @param end       Iterator pointing past the last request.
   * @param callback  See documentation of single-request send().
   *
   * @return true iff the requests were scheduled for sending,
   *         false otherwise (e.g. CarbonRouterInstance was destroyed).
   *
   * Note: the caller is responsible for keeping requests alive until the
   *       callback is called for each of them.
   * Note: the order in which callbacks will be called is undefined, but
   *       it's guaranteed that the callback is called exactly once for
   *       each request.
   */
  template <class InputIt, class F>
  bool send(
      InputIt begin,
      InputIt end,
      F&& callback,
      folly::StringPiece ipAddr = folly::StringPiece());

  CacheClientCounters getStatCounters() noexcept {
    return stats_.getCounters();
  }

  /**
   * Override default proxy assignment.
   */
  void setProxyIndex(size_t proxyIdx) {
    proxyIdx_ = proxyIdx;
  }

  CarbonRouterClient(const CarbonRouterClient<RouterInfo>&) = delete;
  CarbonRouterClient(CarbonRouterClient<RouterInfo>&&) noexcept = delete;
  CarbonRouterClient& operator=(const CarbonRouterClient<RouterInfo>&) = delete;
  CarbonRouterClient& operator=(CarbonRouterClient<RouterInfo>&&) = delete;

  ~CarbonRouterClient() override;

 private:
  std::weak_ptr<CarbonRouterInstance<RouterInfo>> router_;
  ThreadMode mode_;

  // Reference to the vector of proxies inside router_.
  const std::vector<Proxy<RouterInfo>*>& proxies_;
  // The proxy to use when either on FixedRemoteThread or on SameThread mode.
  size_t proxyIdx_{0};
  // Keeps track of proxies with notification pending.
  // Only used for multi-request send() calls.
  std::vector<bool> proxiesToNotify_;

  CacheClientStats stats_;

  /**
   * The user let go of the CarbonRouterClient::Pointer, and the object
   * is pending destruction when all requests complete.
   */
  std::atomic<bool> disconnected_{false};

  /**
   * The ownership is shared between the user and the outstanding requests.
   */
  std::shared_ptr<CarbonRouterClient<RouterInfo>> self_;

  CarbonRouterClient(
      std::shared_ptr<CarbonRouterInstance<RouterInfo>> router,
      size_t maximum_outstanding,
      bool maximum_outstanding_error,
      ThreadMode mode);

  static Pointer create(
      std::shared_ptr<CarbonRouterInstance<RouterInfo>> router,
      size_t maximum_outstanding,
      bool maximum_outstanding_error,
      ThreadMode mode);

  /**
   * Batch send requests.
   *
   * @param nreqs          number of requests to be sent.
   * @param makeNextPreq   proxy request generator.
   * @param failRemaining  will be called if all remaining requests should be
   *                       canceled due to maxOutstandingError_ flag
   */
  template <class F, class G>
  bool sendMultiImpl(size_t nreqs, F&& makeNextPreq, G&& failRemaining);

  void sendRemoteThread(
      std::unique_ptr<ProxyRequestContextWithInfo<RouterInfo>> req,
      bool skipNotification);
  void sendSameThread(
      std::unique_ptr<ProxyRequestContextWithInfo<RouterInfo>> req);

  /**
   * Finds the best proxy to be used to route the request.
   * NOTE: This should only be used when ThreadMode == AffinitizedRemoteThread.
   */
  template <class Request>
  typename std::enable_if<
      ListContains<typename RouterInfo::RoutableRequests, Request>::value,
      std::pair<uint64_t, uint64_t>>::type
  findAffinitizedProxyIdx(const Request& req) const;

  template <class Request>
  typename std::enable_if<
      !ListContains<typename RouterInfo::RoutableRequests, Request>::value,
      std::pair<uint64_t, uint64_t>>::type
  findAffinitizedProxyIdx(const Request& req) const;

  /**
   * Creates the ProxyRequestContext that represents the given request.
   *
   * @param req         The request to be routed.
   * @param callback    The callback function to be called once the reply
   *                    is received.
   * @param ipAddr      The ip address of the caller (can be empty).
   * @param inBatch     Whether or not the given request is part of a batch
   *                    of requests.
   *
   * @return            The ProxyRequestContext.
   */
  template <class Request, class CallbackFunc>
  std::unique_ptr<ProxyRequestContextWithInfo<RouterInfo>>
  makeProxyRequestContext(
      const Request& req,
      CallbackFunc&& callback,
      folly::StringPiece ipAddr,
      bool inBatch);

  bool shouldDelayNotification(size_t batchSize) const;

  friend class CarbonRouterInstance<RouterInfo>;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "CarbonRouterClient-inl.h"
