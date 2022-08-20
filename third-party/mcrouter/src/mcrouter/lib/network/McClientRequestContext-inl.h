/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/fbi/cpp/LogFailure.h"

namespace facebook {
namespace memcache {

template <class Reply>
void McClientRequestContextBase::reply(Reply&& r) {
  assert(
      state() == ReqState::PENDING_REPLY_QUEUE ||
      state() == ReqState::WRITE_QUEUE);
  if (replyType_ != typeid(Reply)) {
    LOG_FAILURE(
        "AsyncMcClient",
        failure::Category::kBrokenLogic,
        "Attempt to forward a reply of a wrong type. Expected '{}', "
        "but received '{}'!",
        replyType_.name(),
        typeid(Reply).name());

    replyErrorImpl(
        carbon::Result::LOCAL_ERROR,
        "Attempt to forward a reply of wrong type.");
    return;
  }

  auto* storage = reinterpret_cast<folly::Optional<Reply>*>(replyStorage_);
  assert(!storage->hasValue());
  storage->emplace(std::move(r));
}

template <class Request>
McClientRequestContextBase::McClientRequestContextBase(
    const Request& request,
    uint64_t reqid,
    mc_protocol_t protocol,
    folly::Optional<ReplyT<Request>>& replyStorage,
    McClientRequestContextQueue& queue,
    InitializerFuncPtr initializer,
    const std::function<void(int pendingDiff, int inflightDiff)>& onStateChange,
    const CodecIdRange& supportedCodecs)
    : reqContext(request, reqid, protocol, supportedCodecs),
      id(reqid),
      queue_(queue),
      replyType_(typeid(ReplyT<Request>)),
      replyStorage_(reinterpret_cast<void*>(&replyStorage)),
      initializer_(std::move(initializer)),
      onStateChange_(onStateChange) {}

template <class Request>
void McClientRequestContext<Request>::replyErrorImpl(
    carbon::Result result,
    folly::StringPiece errorMessage) {
  assert(!replyStorage_.hasValue());
  replyStorage_.assign(
      createReply<Request>(ErrorReply, result, errorMessage.str()));
}

template <class Request>
std::string McClientRequestContext<Request>::getContextTypeStr() const {
  return folly::sformat("RequestT is {}", typeid(Request).name());
}

template <class Request>
typename McClientRequestContext<Request>::Reply
McClientRequestContext<Request>::waitForReply(
    std::chrono::milliseconds timeout) {
  batonWaitTimeout_ = timeout;
  baton_.wait(batonTimeoutHandler_);

  switch (state()) {
    case ReqState::REPLIED_QUEUE:
      // The request was already replied, but we're still waiting for socket
      // write to succeed.
      baton_.reset();
      baton_.wait();
      assert(state() == ReqState::COMPLETE);
      return std::move(replyStorage_.value());
    case ReqState::PENDING_QUEUE:
      // Request wasn't sent to the network yet, reply with timeout.
      queue_.removePending(*this);
      return createReply<Request>(
          ErrorReply, carbon::Result::TIMEOUT, "Client queue timeout");
    case ReqState::PENDING_REPLY_QUEUE:
      // Request was sent to the network, but wasn't replied yet,
      // reply with timeout.
      queue_.removePendingReply(*this);
      return createReply<Request>(
          ErrorReply, carbon::Result::TIMEOUT, "Reply timeout");
    case ReqState::COMPLETE:
      assert(replyStorage_.hasValue());
      return std::move(replyStorage_.value());
    case ReqState::WRITE_QUEUE:
    case ReqState::NONE:
      LOG_FAILURE(
          "AsyncMcClient",
          failure::Category::kBrokenLogic,
          "Unexpected state of request: {}!",
          static_cast<uint64_t>(state()));
  }
  return Reply(carbon::Result::LOCAL_ERROR);
}

template <class Request>
McClientRequestContext<Request>::McClientRequestContext(
    const Request& request,
    uint64_t reqid,
    mc_protocol_t protocol,
    McClientRequestContextQueue& queue,
    McClientRequestContextBase::InitializerFuncPtr func,
    const std::function<void(int pendingDiff, int inflightDiff)>& onStateChange,
    const CodecIdRange& supportedCodecs)
    : McClientRequestContextBase(
          request,
          reqid,
          protocol,
          replyStorage_,
          queue,
          std::move(func),
          onStateChange,
          supportedCodecs),
      requestTraceContext_(request.traceContext()) {}

template <class Reply>
void McClientRequestContextQueue::reply(
    uint64_t id,
    Reply&& r,
    RpcStatsContext rpcStatsContext) {
  // Get the context and erase it from the queue and map.
  McClientRequestContextBase* ctx{nullptr};
  if (outOfOrder_) {
    auto iter = getContextById(id);
    if (iter != set_.end()) {
      ctx = &(*iter);
      if (iter->state() == State::PENDING_REPLY_QUEUE) {
        pendingReplyQueue_.erase(pendingReplyQueue_.iterator_to(*iter));
        set_.erase(iter);
      } else if (iter->state() == State::WRITE_QUEUE) {
        // We didn't get write callback yet, so need to properly handle that.
        set_.erase(iter);
      } else {
        LOG_FAILURE(
            "AsyncMcClient",
            failure::Category::kOther,
            "Received reply for a request in an unexpected state {}!",
            static_cast<uint64_t>(iter->state()));
        return;
      }

      auto oldState = ctx->state();
      ctx->reply(std::move(r));
      ctx->setRpcStatsContext(rpcStatsContext);
      ctx->setState(State::COMPLETE);

      if (oldState == State::PENDING_REPLY_QUEUE) {
        ctx->baton_.post();
      }
    }
  } else {
    // First we're going to receive replies for timed out requests.
    if (!timedOutInitializers_.empty()) {
      timedOutInitializers_.pop();
    } else if (!pendingReplyQueue_.empty()) {
      ctx = &pendingReplyQueue_.front();
      pendingReplyQueue_.pop_front();
    } else if (!writeQueue_.empty()) {
      ctx = &writeQueue_.front();
      writeQueue_.pop_front();
    } else {
      // With old mc_parser it's possible to receive unexpected replies, we need
      // to ignore them. But we need to log this.
      LOG_FAILURE(
          "AsyncMcClient",
          failure::Category::kOther,
          "Received unexpected reply from server!");
    }

    if (ctx) {
      ctx->reply(std::move(r));
      ctx->setRpcStatsContext(rpcStatsContext);
      if (ctx->state() == State::PENDING_REPLY_QUEUE) {
        ctx->setState(State::COMPLETE);
        ctx->baton_.post();
      } else {
        // Move the request to the replied queue.
        ctx->setState(State::REPLIED_QUEUE);
        repliedQueue_.push_back(*ctx);
      }
    }
  }
}
} // namespace memcache
} // namespace facebook
