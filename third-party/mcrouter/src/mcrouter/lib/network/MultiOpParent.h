/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <folly/Optional.h>

#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {

class McServerSession;

/**
 * We use this struct to track the state of multi-op operations
 * for in-order protocols.
 *
 * Consider the request "get a b". This generates four contexts:
 *   block_ctx, a_ctx, b_ctx, end_ctx
 *
 * The purpose of the block_ctx is to prevent any writes until
 * both a and b complete (it's a hack that uses the head of line
 * blocking property of the in-order protocol).
 *
 * a_ctx and b_ctx are the real contexts dispatched to the application,
 * and they both have back pointers to this parent struct.
 *
 * end_ctx is for writing the END message on the wire.
 *
 * Error handling: if either a or b returns an error, we want to
 * turn the entire reply into a single error reply.
 * How it's implemented: if b_ctx returns an error, we'll store the error
 * reply in the parent. Then when unblocked, both a_ctx and b_ctx
 * will check that the parent has a stored error and will not
 * write anything to the transport (same as if 'noreply' was set).
 *
 * Finally the end context will write out the stored error reply.
 */
class MultiOpParent {
 public:
  MultiOpParent(McServerSession& session, uint64_t blockReqid);

  /**
   * Examine the reply result of one of the sub-requests. If it's an error
   * result, inform the caller that this parent will assume responsibility
   * of reporting an error and that the caller should not reply.
   *
   * @return true if the parent assumed ownership of reporting an error.
   *         On true, errorMessage is moved out of.
   */
  bool
  reply(carbon::Result result, uint32_t errorCode, std::string&& errorMessage);

  /**
   * Notify that a sub request is waiting for a reply.
   */
  void recordRequest() {
    ++waiting_;
  }

  /**
   * Notify that we saw a multi-op end sentinel, and create the 'end' context
   * with this id.
   */
  void recordEnd(uint64_t reqid);

  /**
   * @return true if an error was observed
   */
  bool error() const {
    return error_;
  }

 private:
  size_t waiting_{0};
  folly::Optional<McGetReply> reply_;
  bool error_{false};

  McServerSession& session_;
  McServerRequestContext block_;
  folly::Optional<McServerRequestContext> end_;

  void release();
};
} // namespace memcache
} // namespace facebook
