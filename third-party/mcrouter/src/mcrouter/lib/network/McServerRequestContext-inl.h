/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/network/McServerSession.h"
#include "mcrouter/lib/network/WriteBuffer.h"

#ifndef LIBMC_FBTRACE_DISABLE
#include "mcrouter/facebook/ArtilleryTracing.h"
#endif

namespace facebook {
namespace memcache {

template <class Reply>
void McServerRequestContext::reply(
    McServerRequestContext&& ctx,
    Reply&& reply,
    bool flush) {
  replyImpl(std::move(ctx), std::move(reply), nullptr, nullptr, flush);
}

template <class Reply>
void McServerRequestContext::reply(
    McServerRequestContext&& ctx,
    Reply&& reply,
    DestructorFunc destructor,
    void* toDestruct) {
  replyImpl(
      std::move(ctx),
      std::move(reply),
      destructor,
      toDestruct,
      false /* flush */);
}

template <class Reply, class... Args>
typename std::enable_if<carbon::GetLike<
    RequestFromReplyType<Reply, RequestReplyPairs>>::value>::type
McServerRequestContext::replyImpl(
    McServerRequestContext&& ctx,
    Reply&& reply,
    Args&&... args) {
  // On error, multi-get parent may assume responsiblity of replying
  if (ctx.moveReplyToParent(
          *reply.result_ref(),
          *reply.appSpecificErrorCode_ref(),
          std::move(*reply.message_ref()))) {
    replyImpl2(std::move(ctx), Reply(), std::forward<Args>(args)...);
  } else {
    replyImpl2(std::move(ctx), std::move(reply), std::forward<Args>(args)...);
  }
}

template <class Reply, class... Args>
typename std::enable_if<carbon::OtherThan<
    RequestFromReplyType<Reply, RequestReplyPairs>,
    carbon::GetLike<>>::value>::type
McServerRequestContext::replyImpl(
    McServerRequestContext&& ctx,
    Reply&& reply,
    Args&&... args) {
  replyImpl2(std::move(ctx), std::move(reply), std::forward<Args>(args)...);
}

template <class Reply, class SessionType>
void McServerRequestContext::replyImpl2(
    McServerRequestContext&& ctx,
    Reply&& reply,
    DestructorFunc destructor,
    void* toDestruct,
    bool flush) {
#ifndef LIBMC_FBTRACE_DISABLE
  if (FOLLY_UNLIKELY(ctx.isTraced_)) {
    auto tracer = facebook::mcrouter::getCurrentTracer();
    if (FOLLY_LIKELY(tracer != nullptr)) {
      reply.setTraceContext(tracer->sendResponse());
    }
  }
#endif
  ctx.replied_ = true;
  // Note: 'SessionType' being a template parameter allows the use of
  // McServerSession members, otherwise there's a circular dependency preventing
  // concrete use of McServerSession here.
  SessionType* const session = ctx.session_;
  if (toDestruct != nullptr) {
    assert(destructor != nullptr);
  }
  // Call destructor(toDestruct) on error, or pass ownership to write buffer
  std::unique_ptr<void, void (*)(void*)> destructorContainer(
      toDestruct, destructor);

  if (ctx.noReply(reply)) {
    session->reply(nullptr, ctx.reqid_);
    return;
  }

  uint64_t reqid = ctx.reqid_;
  auto wb = session->writeBufs_.get(session->parser_.protocol());
  if (!wb->prepareTyped(
          std::move(ctx),
          std::move(reply),
          std::move(destructorContainer),
          session->compressionCodecMap_,
          session->codecIdRange_,
          session->options_.tcpZeroCopyThresholdBytes)) {
    session->transport_->close();
    return;
  }
  session->reply(std::move(wb), reqid);
  if (FOLLY_UNLIKELY(flush)) {
    session->flushWrites();
  }
}

/**
 * No reply if either:
 *  1) We saw an error (the error will be printed out by the end context),
 *  2) This is a miss, except for lease-get (lease-get misses still have
 *     'LVALUE' replies with the token).
 * Lease-gets are handled in a separate overload below.
 */
template <class Reply>
bool McServerRequestContext::noReply(const Reply& r) const {
  if (noReply_) {
    return true;
  }
  if (!hasParent()) {
    return false;
  }
  return isParentError() || *r.result_ref() != carbon::Result::FOUND;
}

inline bool McServerRequestContext::noReply(const McLeaseGetReply&) const {
  if (noReply_) {
    return true;
  }
  if (!hasParent()) {
    return false;
  }
  return isParentError();
}

template <class T, class Enable = void>
struct HasDispatchTypedRequest {
  static constexpr std::false_type value{};
};

template <class T>
struct HasDispatchTypedRequest<
    T,
    typename std::enable_if<std::is_same<
        decltype(std::declval<T>().dispatchTypedRequest(
            std::declval<CaretMessageInfo>(),
            std::declval<folly::IOBuf>(),
            std::declval<McServerRequestContext>())),
        bool>::value>::type> {
  static constexpr std::true_type value{};
};

template <class OnRequest>
void McServerOnRequestWrapper<OnRequest, List<>>::caretRequestReady(
    const CaretMessageInfo& headerInfo,
    const folly::IOBuf& reqBuf,
    McServerRequestContext&& ctx) {
  dispatchTypedRequestIfDefined(
      headerInfo,
      reqBuf,
      std::move(ctx),
      HasDispatchTypedRequest<OnRequest>::value);
}

} // namespace memcache
} // namespace facebook
