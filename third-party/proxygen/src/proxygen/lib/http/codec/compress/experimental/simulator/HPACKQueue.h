/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/DestructorCheck.h>
#include <proxygen/lib/http/codec/compress/HPACKCodec.h>

#include <deque>
#include <memory>
#include <tuple>

namespace proxygen {

class HPACKQueue : public folly::DestructorCheck {
 public:
  explicit HPACKQueue(HPACKCodec& codec) : codec_(codec) {
  }

  void enqueueHeaderBlock(uint32_t seqn,
                          std::unique_ptr<folly::IOBuf> block,
                          size_t length,
                          HPACK::StreamingCallback* streamingCb,
                          bool oooOk) {
    if (seqn < nextSeqn_) {
      streamingCb->onDecodeError(HPACK::DecodeError::BAD_SEQUENCE_NUMBER);
      return;
    }
    if (nextSeqn_ == seqn) {
      // common case, decode immediately
      if (decodeBlock(seqn,
                      std::move(block),
                      length,
                      streamingCb,
                      false /* in order */)) {
        return;
      }
      drainQueue();
    } else {
      // there's a gap, have to queue
      auto it = queue_.begin();
      while (it != queue_.end()) {
        auto qSeqn = std::get<0>(*it);
        if (seqn == qSeqn) {
          streamingCb->onDecodeError(HPACK::DecodeError::BAD_SEQUENCE_NUMBER);
          return;
        } else if (seqn < qSeqn) {
          break;
        }
        it++;
      }
      if (oooOk) {
        // out-of-order allowed.  Decode the block, but make an empty entry in
        // the queue
        if (decodeBlock(
                seqn, std::move(block), length, streamingCb, true /* ooo */)) {
          return;
        }
        length = 0;
        streamingCb = nullptr;
      } else {
        holBlockCount_++;
      }
      VLOG(5) << "queued block=" << seqn << " len=" << length
              << " placeholder=" << int32_t(oooOk);
      queuedBytes_ += length;
      queue_.emplace(it, seqn, std::move(block), length, streamingCb);
    }
  }

  [[nodiscard]] uint64_t getHolBlockCount() const {
    return holBlockCount_;
  }

  [[nodiscard]] uint64_t getQueuedBytes() const {
    return queuedBytes_;
  }

 private:
  // Returns true if this object was destroyed by its callback.  Callers
  // should check the result and immediately return.
  bool decodeBlock(int32_t seqn,
                   std::unique_ptr<folly::IOBuf> block,
                   size_t length,
                   HPACK::StreamingCallback* cb,
                   bool ooo) {
    if (length > 0) {
      VLOG(5) << "decodeBlock for block=" << seqn << " len=" << length;
      folly::io::Cursor c(block.get());
      folly::DestructorCheck::Safety safety(*this);
      codec_.decodeStreaming(c, length, cb);
      if (safety.destroyed()) {
        return true;
      }
    }
    if (!ooo) {
      nextSeqn_++;
    }
    return false;
  }

  void drainQueue() {
    while (!queue_.empty() && nextSeqn_ == std::get<0>(queue_.front())) {
      auto& next = queue_.front();
      auto length = std::get<2>(next);
      if (decodeBlock(std::get<0>(next),
                      std::move(std::get<1>(next)),
                      length,
                      std::get<3>(next),
                      false /* in order */)) {
        return;
      }
      DCHECK_LE(length, queuedBytes_);
      queuedBytes_ -= length;
      queue_.pop_front();
    }
  }

  size_t nextSeqn_{0};
  uint64_t holBlockCount_{0};
  uint64_t queuedBytes_{0};
  std::deque<std::tuple<uint32_t,
                        std::unique_ptr<folly::IOBuf>,
                        size_t,
                        HPACK::StreamingCallback*>>
      queue_;
  HPACKCodec& codec_;
};

} // namespace proxygen
