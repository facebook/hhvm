/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/network/FBTrace.h"
#include "mcrouter/lib/network/RpcStatsContext.h"

namespace facebook {
namespace memcache {

template <class Request>
ReplyT<Request> AsyncMcClientImpl::sendSync(
    const Request& request,
    std::chrono::milliseconds timeout,
    RpcStatsContext* rpcContext) {
  DestructorGuard dg(this);

  assert(folly::fibers::onFiber());

  if (maxPending_ != 0 && queue_.getPendingRequestCount() >= maxPending_) {
    return createReply<Request>(
        ErrorReply,
        folly::sformat(
            "Max pending requests ({}) reached for destination \"{}\".",
            maxPending_,
            connectionOptions_.accessPoint->toHostPortString()));
  }

  McClientRequestContext<Request> ctx(
      request,
      nextMsgId_,
      connectionOptions_.accessPoint->getProtocol(),
      queue_,
      [](ParserT& parser) { parser.expectNext<Request>(); },
      requestStatusCallbacks_.onStateChange,
      supportedCompressionCodecs_);
  sendCommon(ctx);

  // Wait for the reply.
  auto reply = ctx.waitForReply(timeout);

  if (rpcContext) {
    *rpcContext = ctx.getRpcStatsContext();
  }

  // Schedule next writer loop, in case we didn't before
  // due to max inflight requests limit.
  scheduleNextWriterLoop();

  return reply;
}

template <class Reply>
void AsyncMcClientImpl::replyReady(
    Reply&& r,
    uint64_t reqId,
    RpcStatsContext rpcStatsContext) {
  assert(connectionState_ == ConnectionState::Up);
  DestructorGuard dg(this);

  queue_.reply(reqId, std::move(r), rpcStatsContext);
}

} // namespace memcache
} // namespace facebook
