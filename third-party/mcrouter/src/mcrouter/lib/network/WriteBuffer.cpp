/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Conv.h>
#include <folly/String.h>

#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/mc/protocol.h"
#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/MultiOpParent.h"

namespace facebook {
namespace memcache {

WriteBuffer::WriteBuffer(mc_protocol_t protocol) : protocol_(protocol) {
  switch (protocol_) {
    case mc_ascii_protocol:
      new (&asciiReply_) AsciiSerializedReply;
      break;

    case mc_caret_protocol:
      new (&caretReply_) CaretSerializedMessage;
      break;

    default:
      CHECK(false) << "Unknown protocol";
  }
}

WriteBuffer::~WriteBuffer() {
  switch (protocol_) {
    case mc_ascii_protocol:
      asciiReply_.~AsciiSerializedReply();
      break;

    case mc_caret_protocol:
      caretReply_.~CaretSerializedMessage();
      break;

    default:
      CHECK(false) << "Unknown protocol";
  }
}

void WriteBuffer::clear() {
  ctx_.reset();
  destructor_.reset();
  isEndOfBatch_ = false;
  typeId_ = 0;
  zeroCopyPendingNotifications_ = 0;

  switch (protocol_) {
    case mc_ascii_protocol:
      asciiReply_.clear();
      break;

    case mc_caret_protocol:
      caretReply_.clear();
      break;

    default:
      CHECK(false);
  }
}

bool WriteBuffer::noReply() const {
  return ctx_.has_value() && ctx_->hasParent() && ctx_->parent().error();
}

bool WriteBuffer::isSubRequest() const {
  return ctx_.has_value() && (ctx_->hasParent() || ctx_->isEndContext());
}

bool WriteBuffer::isEndContext() const {
  return ctx_.has_value() ? ctx_->isEndContext() : false;
}

WriteBuffer::List& WriteBufferQueue::initFreeStack(
    mc_protocol_t protocol) noexcept {
  assert(protocol == mc_ascii_protocol || protocol == mc_caret_protocol);

  static thread_local WriteBuffer::List freeBuffers[mc_nprotocols];
  return freeBuffers[static_cast<size_t>(protocol)];
}

} // namespace memcache
} // namespace facebook
