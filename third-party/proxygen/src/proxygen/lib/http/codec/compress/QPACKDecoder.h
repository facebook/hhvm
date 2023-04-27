/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/DestructorCheck.h>
#include <map>
#include <proxygen/lib/http/codec/compress/HPACKDecodeBuffer.h>
#include <proxygen/lib/http/codec/compress/HPACKDecoderBase.h>
#include <proxygen/lib/http/codec/compress/HeaderCodec.h>
#include <proxygen/lib/http/codec/compress/QPACKContext.h>

namespace proxygen {

class QPACKDecoder
    : public HPACKDecoderBase
    , public QPACKContext
    , public folly::DestructorCheck {
 public:
  explicit QPACKDecoder(
      uint32_t tableSize = HPACK::kTableSize,
      uint32_t maxUncompressed = HeaderCodec::kMaxUncompressed)
      : HPACKDecoderBase(tableSize, maxUncompressed),
        QPACKContext(tableSize, false /* don't track references */) {
  }

  void decodeStreaming(uint64_t streamId,
                       std::unique_ptr<folly::IOBuf> block,
                       uint32_t totalBytes,
                       HPACK::StreamingCallback* streamingCb);

  HPACK::DecodeError decodeEncoderStream(std::unique_ptr<folly::IOBuf> buf);

  HPACK::DecodeError encoderStreamEnd();

  std::unique_ptr<folly::IOBuf> encodeInsertCountInc();

  std::unique_ptr<folly::IOBuf> encodeHeaderAck(uint64_t streamId) const;

  std::unique_ptr<folly::IOBuf> encodeCancelStream(uint64_t streamId);

  uint64_t getHolBlockCount() const {
    return holBlockCount_;
  }

  uint64_t getQueuedBytes() const {
    return queuedBytes_;
  }

  void setMaxBlocking(uint32_t maxBlocking) {
    maxBlocking_ = maxBlocking;
  }

  void setHeaderTableMaxSize(uint32_t maxSize) {
    CHECK(maxTableSize_ == 0 || maxTableSize_ == maxSize)
        << "Cannot change non-zero max header table size, "
           "maxTableSize_="
        << maxTableSize_ << " maxSize=" << maxSize;
    HPACKDecoderBase::setHeaderTableMaxSize(table_, maxSize);
  }

 private:
  bool isValid(bool isStatic, uint64_t index, bool aboveBase);

  uint32_t decodePrefix(HPACKDecodeBuffer& dbuf);

  void decodeStreamingImpl(uint32_t requiredInsertCount,
                           uint32_t consumed,
                           HPACKDecodeBuffer& dbuf,
                           HPACK::StreamingCallback* streamingCb);

  uint32_t decodeHeaderQ(HPACKDecodeBuffer& dbuf,
                         HPACK::StreamingCallback* streamingCb);

  uint32_t decodeIndexedHeaderQ(HPACKDecodeBuffer& dbuf,
                                uint32_t prefixLength,
                                bool aboveBase,
                                HPACK::StreamingCallback* streamingCb,
                                headers_t* emitted);

  uint32_t decodeLiteralHeaderQ(HPACKDecodeBuffer& dbuf,
                                bool indexing,
                                bool nameIndexed,
                                uint8_t prefixLength,
                                bool aboveBase,
                                HPACK::StreamingCallback* streamingCb);

  void decodeEncoderStreamInstruction(HPACKDecodeBuffer& dbuf);

  void enqueueHeaderBlock(uint64_t streamId,
                          uint32_t requiredInsertCount,
                          uint32_t baseIndex,
                          uint32_t consumed,
                          std::unique_ptr<folly::IOBuf> block,
                          size_t length,
                          HPACK::StreamingCallback* streamingCb);

  struct PendingBlock {
    PendingBlock(uint64_t sid,
                 uint32_t bi,
                 uint32_t l,
                 uint32_t cons,
                 std::unique_ptr<folly::IOBuf> b,
                 HPACK::StreamingCallback* c)
        : streamID(sid),
          baseIndex(bi),
          length(l),
          consumed(cons),
          block(std::move(b)),
          cb(c) {
    }
    uint64_t streamID;
    uint32_t baseIndex;
    uint32_t length;
    uint32_t consumed;
    std::unique_ptr<folly::IOBuf> block;
    HPACK::StreamingCallback* cb;
  };

  // Returns true if this object was destroyed by its callback.  Callers
  // should check the result and immediately return.
  bool decodeBlock(uint32_t requiredInsertCount, const PendingBlock& pending);

  void drainQueue();
  void errorQueue();

  uint32_t maxBlocking_{HPACK::kDefaultBlocking};
  uint32_t baseIndex_{0};
  uint32_t lastAcked_{0};
  uint32_t holBlockCount_{0};
  uint32_t pendingEncoderBytes_{0};
  uint64_t queuedBytes_{0};
  std::multimap<uint32_t, PendingBlock> queue_;

  // This holds the state of a partially decoded literal insert on the control
  // stream
  struct Partial {
    enum { NAME, VALUE } state{NAME};
    uint32_t consumed;
    HPACKHeader header;
  };
  Partial partial_;
  folly::IOBufQueue ingress_{folly::IOBufQueue::cacheChainLength()};
};

} // namespace proxygen
