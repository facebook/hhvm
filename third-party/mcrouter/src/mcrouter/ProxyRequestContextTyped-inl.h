/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/Proxy.h"
#include "mcrouter/lib/McKey.h"
#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/fbi/cpp/TypeList.h"
#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

/**
 * Implementation class for storing the callback along with the context.
 */
template <class RouterInfo, class Request, class F>
class ProxyRequestContextTypedWithCallback
    : public ProxyRequestContextTyped<RouterInfo, Request> {
 public:
  ProxyRequestContextTypedWithCallback(
      Proxy<RouterInfo>& pr,
      const Request& req,
      F&& f,
      ProxyRequestPriority priority__)
      : ProxyRequestContextTyped<RouterInfo, Request>(pr, req, priority__),
        f_(std::forward<F>(f)) {}

 protected:
  void sendReplyImpl(ReplyT<Request>&& reply) final {
    auto req = this->typedRequest();
    fiber_local<RouterInfo>::runWithoutLocals(
        [this, req, &reply]() { f_(*this, *req, std::move(reply)); });
  }

 private:
  F f_;
};

constexpr const char* kCommandNotSupportedStr = "Command not supported";

template <class RouterInfo, class Request>
bool precheckKey(
    ProxyRequestContextTyped<RouterInfo, Request>& preq,
    const Request& req) {
  constexpr bool kIsMemcacheRequest =
      ListContains<McRequestList, Request>::value;

  auto key = req.key_ref()->fullKey();
  auto err = isKeyValid<kIsMemcacheRequest>(key);
  if (err != mc_req_err_valid) {
    ReplyT<Request> reply(carbon::Result::LOCAL_ERROR);
    carbon::setMessageIfPresent(reply, mc_req_err_to_string(err));
    preq.sendReply(std::move(reply));
    return false;
  }
  return true;
}

// Following methods validate the request and return true if it's correct,
// otherwise they reply it with error and return false;

template <class RouterInfo, class Request>
bool precheckRequest(
    ProxyRequestContextTyped<RouterInfo, Request>& preq,
    const Request& req) {
  return precheckKey(preq, req);
}

template <class RouterInfo>
bool precheckRequest(
    ProxyRequestContextTyped<RouterInfo, McStatsRequest>&,
    const McStatsRequest&) {
  return true;
}

template <class RouterInfo>
bool precheckRequest(
    ProxyRequestContextTyped<RouterInfo, McVersionRequest>&,
    const McVersionRequest&) {
  return true;
}

template <class RouterInfo>
bool precheckRequest(
    ProxyRequestContextTyped<RouterInfo, McShutdownRequest>& preq,
    const McShutdownRequest&) {
  // Return error (pretend to not even understand the protocol)
  preq.sendReply(carbon::Result::BAD_COMMAND);
  return false;
}

template <class RouterInfo>
bool precheckRequest(
    ProxyRequestContextTyped<RouterInfo, McFlushReRequest>& preq,
    const McFlushReRequest&) {
  // Return 'Not supported' message
  McFlushReReply reply(carbon::Result::LOCAL_ERROR);
  carbon::setMessageIfPresent(reply, kCommandNotSupportedStr);
  preq.sendReply(std::move(reply));
  return false;
}

template <class RouterInfo>
bool precheckRequest(
    ProxyRequestContextTyped<RouterInfo, McFlushAllRequest>& preq,
    const McFlushAllRequest&) {
  if (!preq.proxy().getRouterOptions().enable_flush_cmd) {
    McFlushAllReply reply(carbon::Result::LOCAL_ERROR);
    carbon::setMessageIfPresent(reply, "Command disabled");
    preq.sendReply(std::move(reply));
    return false;
  }
  return true;
}

} // namespace detail

template <class RouterInfo, class Request>
void ProxyRequestContextTyped<RouterInfo, Request>::sendReply(
    ReplyT<Request>&& reply) {
  if (FOLLY_UNLIKELY(this->recording())) {
    return;
  }

  if (FOLLY_UNLIKELY(this->replied_)) {
    return;
  }
  this->replied_ = true;
  auto result = *reply.result_ref();

  sendReplyImpl(std::move(reply));
  clearTypedRequest();

  auto& stats = this->proxy().stats();
  stats.increment(request_replied_stat);
  stats.increment(request_replied_count_stat);
  if (FOLLY_UNLIKELY(isErrorResult(result))) {
    stats.increment(request_error_stat);
    stats.increment(request_error_count_stat);
  } else {
    stats.increment(request_success_stat);
    stats.increment(request_success_count_stat);
  }
}

template <class RouterInfo, class Request>
void ProxyRequestContextTyped<RouterInfo, Request>::startProcessing() {
  std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>> self(this);

  if (!detail::precheckRequest(*this, *typedRequest())) {
    return;
  }

  if (this->proxy_.beingDestroyed()) {
    /* We can't process this, since 1) we destroyed the config already,
       and 2) the clients are winding down, so we wouldn't get any
       meaningful response back anyway. */
    LOG(ERROR) << "Outstanding request on a proxy that's being destroyed";
    sendReply(ReplyT<Request>(carbon::Result::UNKNOWN));
    return;
  }

  this->proxy_.dispatchRequest(*typedRequest(), std::move(self));
}

template <class RouterInfo, class Request>
std::shared_ptr<ProxyRequestContextTyped<RouterInfo, Request>>
ProxyRequestContextTyped<RouterInfo, Request>::process(
    std::unique_ptr<Type> preq,
    std::shared_ptr<const ProxyConfig<RouterInfo>> config) {
  preq->config_ = std::move(config);
  return std::shared_ptr<Type>(
      preq.release(),
      /* Note: we want to delete on main context here since the destructor
         can do complicated things, like finalize stats entry and
         destroy a stale config.  There might not be enough stack space
         for these operations. */
      [](ProxyRequestContext* ctx) {
        folly::fibers::runInMainContext([ctx] { delete ctx; });
      });
}

template <class RouterInfo, class Request, class F>
std::unique_ptr<ProxyRequestContextTyped<RouterInfo, Request>>
createProxyRequestContext(
    Proxy<RouterInfo>& pr,
    const Request& req,
    F&& f,
    ProxyRequestPriority priority) {
  using Type =
      detail::ProxyRequestContextTypedWithCallback<RouterInfo, Request, F>;
  return std::make_unique<Type>(pr, req, std::forward<F>(f), priority);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
