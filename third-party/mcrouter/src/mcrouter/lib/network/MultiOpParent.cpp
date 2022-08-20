/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MultiOpParent.h"

namespace facebook {
namespace memcache {

MultiOpParent::MultiOpParent(McServerSession& session, uint64_t blockReqid)
    : session_(session), block_(session, blockReqid, true /* noReply */) {}

bool MultiOpParent::reply(
    carbon::Result result,
    uint32_t errorCode,
    std::string&& errorMessage) {
  bool stolen = false;
  // If not a hit or a miss, and we didn't store a reply yet,
  // take ownership of the error reply and tell caller not to reply
  if (!(result == carbon::Result::FOUND ||
        result == carbon::Result::NOTFOUND) &&
      !reply_.has_value()) {
    stolen = true;
    error_ = true;
    reply_.emplace(result);
    reply_->message_ref() = std::move(errorMessage);
    reply_->appSpecificErrorCode_ref() = errorCode;
  }

  assert(waiting_ > 0);
  --waiting_;

  if (!waiting_ && end_.has_value()) {
    release();
  }

  return stolen;
}

void MultiOpParent::recordEnd(uint64_t reqid) {
  end_ = McServerRequestContext(
      session_,
      reqid,
      false /* noReply */,
      nullptr /* multiOpParent */,
      true /* isEndContext */);
  if (!waiting_) {
    release();
  }
}

void MultiOpParent::release() {
  if (!reply_.has_value()) {
    reply_.emplace(carbon::Result::FOUND);
  }
  McServerRequestContext::reply(std::move(*end_), std::move(*reply_));
  // It doesn't really matter what reply type we use for the multi-op
  // blocking context
  McServerRequestContext::reply(std::move(block_), McGetReply());
}
} // namespace memcache
} // namespace facebook
