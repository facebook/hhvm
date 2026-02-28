/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/Request.h>

#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/network/CarbonMessageDispatcher.h"
#include "mcrouter/lib/network/CaretHeader.h"
#include "mcrouter/lib/network/FBTrace.h"

namespace facebook {
namespace memcache {
void markContextAsTraced(McServerRequestContext& ctx);
} // namespace memcache
} // namespace facebook

namespace carbon {

template <class OnRequest, class RequestList>
class CarbonRequestHandler : public facebook::memcache::CarbonMessageDispatcher<
                                 RequestList,
                                 CarbonRequestHandler<OnRequest, RequestList>,
                                 facebook::memcache::McServerRequestContext&&> {
 public:
  template <class... Args>
  explicit CarbonRequestHandler(Args&&... args)
      : onRequest_(std::make_unique<OnRequest>(std::forward<Args>(args)...)) {}

  template <class Request>
  void onRequest(
      facebook::memcache::McServerRequestContext&& ctx,
      Request&& req) {
    onRequestImpl(
        std::move(ctx),
        std::move(req),
        nullptr /* headerInfo */,
        nullptr /* reqBuffer */,
        carbon::detail::CanHandleRequest::value<Request, OnRequest>());
  }

  template <class Request>
  void onRequest(
      facebook::memcache::McServerRequestContext&& ctx,
      Request&& req,
      const facebook::memcache::CaretMessageInfo& headerInfo,
      const folly::IOBuf& reqBuf) {
    onRequestImpl(
        std::move(ctx),
        std::move(req),
        &headerInfo,
        &reqBuf,
        carbon::detail::CanHandleRequest::value<Request, OnRequest>());
  }

 private:
  std::unique_ptr<OnRequest> onRequest_;

  template <class Request>
  void onRequestImpl(
      facebook::memcache::McServerRequestContext&& ctx,
      Request&& req,
      const facebook::memcache::CaretMessageInfo* headerInfo,
      const folly::IOBuf* reqBuf,
      std::true_type) {
    if (FOLLY_UNLIKELY(
            !req.traceContext().empty() &&
            facebook::mcrouter::traceCheckRateLimit())) {
      onRequestImplWithTracingEnabled(
          std::move(ctx), std::move(req), headerInfo, reqBuf);
      return;
    }
    callOnRequest(
        std::move(ctx),
        std::move(req),
        headerInfo,
        reqBuf,
        carbon::detail::CanHandleRequestWithBuffer::
            value<Request, OnRequest>());
  }

  template <class Request>
  void onRequestImpl(
      facebook::memcache::McServerRequestContext&&,
      Request&&,
      const facebook::memcache::CaretMessageInfo*,
      const folly::IOBuf*,
      std::false_type) {
    facebook::memcache::throwRuntime(
        "onRequest for {} not defined", typeid(Request).name());
  }

  template <class Request>
  FOLLY_NOINLINE void onRequestImplWithTracingEnabled(
      facebook::memcache::McServerRequestContext&& ctx,
      Request&& req,
      const facebook::memcache::CaretMessageInfo* headerInfo,
      const folly::IOBuf* reqBuf) {
#ifndef LIBMC_FBTRACE_DISABLE
    folly::RequestContextScopeGuard requestContextGuard;
    auto tracingData = facebook::mcrouter::traceRequestReceived(
        req.traceContext(), Request::name);
    if (tracingData != nullptr) {
      // Mark the context as being traced by Artillery
      markContextAsTraced(ctx);
      tracingData->startCounters();
    }
#endif
    callOnRequest(
        std::move(ctx),
        std::move(req),
        headerInfo,
        reqBuf,
        carbon::detail::CanHandleRequestWithBuffer::
            value<Request, OnRequest>());
  }

  template <class Request>
  void callOnRequest(
      facebook::memcache::McServerRequestContext&& ctx,
      Request&& req,
      const facebook::memcache::CaretMessageInfo* headerInfo,
      const folly::IOBuf* reqBuf,
      std::true_type) {
    onRequest_->onRequest(std::move(ctx), std::move(req), headerInfo, reqBuf);
  }

  template <class Request>
  void callOnRequest(
      facebook::memcache::McServerRequestContext&& ctx,
      Request&& req,
      const facebook::memcache::CaretMessageInfo*,
      const folly::IOBuf*,
      std::false_type) {
    onRequest_->onRequest(std::move(ctx), std::move(req));
  }
};

} // namespace carbon
