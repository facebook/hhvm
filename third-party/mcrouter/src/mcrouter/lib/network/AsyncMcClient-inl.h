/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace facebook {
namespace memcache {

inline AsyncMcClient::AsyncMcClient(
    folly::EventBase& eventBase,
    ConnectionOptions options)
    : AsyncMcClient(eventBase.getVirtualEventBase(), options) {}

inline AsyncMcClient::AsyncMcClient(
    folly::VirtualEventBase& eventBase,
    ConnectionOptions options)
    : base_(AsyncMcClientImpl::create(eventBase, std::move(options))) {}

inline void AsyncMcClient::closeNow() {
  base_->closeNow();
}

inline void AsyncMcClient::setConnectionStatusCallbacks(
    ConnectionStatusCallbacks callbacks) {
  base_->setConnectionStatusCallbacks(std::move(callbacks));
}

inline void AsyncMcClient::setRequestStatusCallbacks(
    RequestStatusCallbacks callbacks) {
  base_->setRequestStatusCallbacks(std::move(callbacks));
}

inline void AsyncMcClient::setAuthorizationCallbacks(
    AuthorizationCallbacks callbacks) {
  base_->setAuthorizationCallbacks(std::move(callbacks));
}

template <class Request>
ReplyT<Request> AsyncMcClient::sendSync(
    const Request& request,
    std::chrono::milliseconds timeout,
    RpcStatsContext* rpcContext) {
  return base_->sendSync(request, timeout, rpcContext);
}

inline void AsyncMcClient::setThrottle(size_t maxInflight, size_t maxPending) {
  base_->setThrottle(maxInflight, maxPending);
}

inline typename Transport::RequestQueueStats
AsyncMcClient::getRequestQueueStats() const {
  return base_->getRequestQueueStats();
}

inline void AsyncMcClient::updateTimeoutsIfShorter(
    std::chrono::milliseconds connectTimeout,
    std::chrono::milliseconds writeTimeout) {
  base_->updateTimeoutsIfShorter(connectTimeout, writeTimeout);
}

inline const folly::AsyncTransportWrapper* AsyncMcClient::getTransport() const {
  return base_->getTransport();
}

inline double AsyncMcClient::getRetransmitsPerKb() {
  return base_->getRetransmitsPerKb();
}

/* static */ inline constexpr bool AsyncMcClient::isCompatible(
    mc_protocol_t protocol) {
  return protocol == mc_ascii_protocol || protocol == mc_caret_protocol;
}

} // namespace memcache
} // namespace facebook
