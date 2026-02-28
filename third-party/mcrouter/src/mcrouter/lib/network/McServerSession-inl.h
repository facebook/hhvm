/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/MultiOpParent.h"

namespace facebook {
namespace memcache {

template <class Request>
void McServerSession::asciiRequestReady(
    Request&& req,
    carbon::Result result,
    bool noreply) {
  DestructorGuard dg(this);

  using Reply = ReplyT<Request>;

  assert(parser_.protocol() == mc_ascii_protocol);
  assert(!parser_.outOfOrder());

  if (state_ != STREAMING) {
    return;
  }

  if (carbon::GetLike<Request>::value && !currentMultiop_) {
    currentMultiop_ = std::make_shared<MultiOpParent>(*this, tailReqid_++);
  }
  uint64_t reqid;
  reqid = tailReqid_++;

  McServerRequestContext ctx(*this, reqid, noreply, currentMultiop_);

  ctx.asciiKey().emplace(req.key_ref()->raw().cloneOneAsValue());

  if (result == carbon::Result::BAD_KEY) {
    McServerRequestContext::reply(
        std::move(ctx), Reply(carbon::Result::BAD_KEY));
  } else {
    try {
      onRequest_->requestReady(std::move(ctx), std::move(req));
    } catch (...) {
      McServerRequestContext::reply(
          std::move(ctx), Reply(carbon::Result::REMOTE_ERROR));
    }
  }
}

} // namespace memcache
} // namespace facebook
