/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <utility>

#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {

template <typename Request>
using ReplyT = typename Request::reply_type;

/**
 * Type tags for Reply constructors.
 */
enum DefaultReplyT { DefaultReply };
enum ErrorReplyT { ErrorReply };
enum RemoteErrorReplyT { RemoteErrorReply };
enum TkoReplyT { TkoReply };
enum BusyReplyT { BusyReply };
enum DeadlineExceededReplyT { DeadlineExceededReply };

template <class Request>
ReplyT<Request>
createReply(DefaultReplyT, const Request&, carbon::UpdateLikeT<Request> = 0) {
  return ReplyT<Request>(carbon::Result::NOTSTORED);
}

template <class Request>
ReplyT<Request> createReply(
    DefaultReplyT,
    const Request&,
    carbon::OtherThanT<Request, carbon::UpdateLike<>> = 0) {
  return ReplyT<Request>(carbon::Result::NOTFOUND);
}

template <class Request>
ReplyT<Request> createReply(ErrorReplyT) {
  return ReplyT<Request>(carbon::Result::LOCAL_ERROR);
}

template <class Request>
ReplyT<Request> createReply(RemoteErrorReplyT) {
  return ReplyT<Request>(carbon::Result::REMOTE_ERROR);
}

template <class Request>
ReplyT<Request> createReply(RemoteErrorReplyT, std::string errorMessage) {
  ReplyT<Request> reply(carbon::Result::REMOTE_ERROR);
  carbon::setMessageIfPresent(reply, std::move(errorMessage));
  return reply;
}

template <class Request>
ReplyT<Request> createReply(DeadlineExceededReplyT) {
  return ReplyT<Request>(carbon::Result::DEADLINE_EXCEEDED);
}

template <class Request>
ReplyT<Request> createReply(ErrorReplyT, std::string errorMessage) {
  ReplyT<Request> reply(carbon::Result::LOCAL_ERROR);
  carbon::setMessageIfPresent(reply, std::move(errorMessage));
  return reply;
}

template <class Request>
ReplyT<Request>
createReply(ErrorReplyT, carbon::Result result, std::string errorMessage) {
  assert(isErrorResult(result));
  ReplyT<Request> reply(result);
  carbon::setMessageIfPresent(reply, std::move(errorMessage));
  return reply;
}

template <class Request>
ReplyT<Request> createReply(TkoReplyT) {
  return ReplyT<Request>(carbon::Result::TKO);
}

template <class Request>
ReplyT<Request> createReply(TkoReplyT, std::string errorMessage) {
  ReplyT<Request> reply(carbon::Result::TKO);
  carbon::setMessageIfPresent(reply, std::move(errorMessage));
  return reply;
}

template <class Request>
ReplyT<Request> createReply(BusyReplyT) {
  return ReplyT<Request>(carbon::Result::BUSY);
}

template <class Reply>
void setReplyResultAndMessage(
    Reply& reply,
    carbon::Result res,
    std::string errorMessage) {
  reply.result_ref() = res;
  carbon::setMessageIfPresent(reply, std::move(errorMessage));
}

} // namespace memcache
} // namespace facebook
