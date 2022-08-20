/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/Proxy.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/RequestLoggerContext.h"
#include "mcrouter/lib/carbon/Stats.h"
#include "mcrouter/options.h"
#include "mcrouter/stats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
template <class Request>
void ProxyRequestLogger<RouterInfo>::log(
    const RequestLoggerContext& loggerContext) {
  if (loggerContext.requestClass.isNormal()) {
    proxy_.requestStats().template bump<Request>(
        carbon::RouterStatTypes::Outgoing);
  }
  proxy_.requestStats().template bump<Request>(
      carbon::RouterStatTypes::AllOutgoing);

  logError(loggerContext.replyResult, loggerContext.requestClass);

  const auto durationUs = loggerContext.endTimeUs - loggerContext.startTimeUs;
  proxy_.stats().durationUs().insertSample(durationUs);
  logDurationByRequestType<Request>(durationUs);
}

template <class RouterInfo>
template <class Request>
void ProxyRequestLogger<RouterInfo>::logDurationByRequestType(
    uint64_t durationUs,
    carbon::GetLikeT<Request>) {
  proxy_.stats().durationGetUs().insertSample(durationUs);
}

template <class RouterInfo>
template <class Request>
void ProxyRequestLogger<RouterInfo>::logDurationByRequestType(
    uint64_t durationUs,
    carbon::UpdateLikeT<Request>) {
  proxy_.stats().durationUpdateUs().insertSample(durationUs);
}

template <class RouterInfo>
template <class Request>
void ProxyRequestLogger<RouterInfo>::logDurationByRequestType(
    uint64_t /* durationUs */,
    carbon::OtherThanT<Request, carbon::GetLike<>, carbon::UpdateLike<>>) {}

#define REQUEST_CLASS_ERROR_STATS(proxy, ERROR, reqClass)     \
  do {                                                        \
    if (reqClass.isNormal()) {                                \
      proxy.stats().increment(result_##ERROR##_stat);         \
      proxy.stats().increment(result_##ERROR##_count_stat);   \
    }                                                         \
    proxy.stats().increment(result_##ERROR##_all_stat);       \
    proxy.stats().increment(result_##ERROR##_all_count_stat); \
  } while (0)

template <class RouterInfo>
void ProxyRequestLogger<RouterInfo>::logError(
    carbon::Result result,
    RequestClass reqClass) {
  if (isErrorResult(result)) {
    REQUEST_CLASS_ERROR_STATS(proxy_, error, reqClass);
  }
  if (isConnectErrorResult(result)) {
    REQUEST_CLASS_ERROR_STATS(proxy_, connect_error, reqClass);
  }
  if (isConnectTimeoutResult(result)) {
    REQUEST_CLASS_ERROR_STATS(proxy_, connect_timeout, reqClass);
  }
  if (isDataTimeoutResult(result)) {
    REQUEST_CLASS_ERROR_STATS(proxy_, data_timeout, reqClass);
  }
  if (isRedirectResult(result)) {
    REQUEST_CLASS_ERROR_STATS(proxy_, busy, reqClass);
  }
  if (isTkoResult(result)) {
    REQUEST_CLASS_ERROR_STATS(proxy_, tko, reqClass);
  }
  if (isRemoteErrorResult(result)) {
    REQUEST_CLASS_ERROR_STATS(proxy_, remote_error, reqClass);
  }
  if (isLocalErrorResult(result)) {
    REQUEST_CLASS_ERROR_STATS(proxy_, local_error, reqClass);
  }
  if (isClientErrorResult(result)) {
    REQUEST_CLASS_ERROR_STATS(proxy_, client_error, reqClass);
  }
  if (isDeadlineExceededResult(result)) {
    REQUEST_CLASS_ERROR_STATS(proxy_, deadline_exceeded_error, reqClass);
  }
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
