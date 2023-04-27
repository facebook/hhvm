/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/QPACKDecoder.h>

#include <proxygen/lib/http/codec/compress/HPACKEncodeBuffer.h>

using folly::io::Cursor;

namespace {
const uint32_t kGrowth = 100;
}

namespace proxygen {

// Blocking implementation - may queue
void QPACKDecoder::decodeStreaming(uint64_t streamID,
                                   std::unique_ptr<folly::IOBuf> block,
                                   uint32_t totalBytes,
                                   HPACK::StreamingCallback* streamingCb) {
  Cursor cursor(block.get());
  HPACKDecodeBuffer dbuf(cursor, totalBytes, maxUncompressed_);
  err_ = HPACK::DecodeError::NONE;
  uint32_t requiredInsertCount = decodePrefix(dbuf);
  if (requiredInsertCount > table_.getInsertCount()) {
    VLOG(5) << "requiredInsertCount=" << requiredInsertCount
            << " > insertCount=" << table_.getInsertCount() << ", queuing";
    if (queue_.size() >= maxBlocking_) {
      VLOG(2) << "QPACK queue full size=" << queue_.size()
              << " maxBlocking_=" << maxBlocking_;
      err_ = HPACK::DecodeError::TOO_MANY_BLOCKING;
      completeDecode(HeaderCodec::Type::QPACK, streamingCb, 0, 0, 0, false);
    } else {
      folly::IOBufQueue q;
      q.append(std::move(block));
      q.trimStart(dbuf.consumedBytes());
      enqueueHeaderBlock(streamID,
                         requiredInsertCount,
                         baseIndex_,
                         dbuf.consumedBytes(),
                         q.move(),
                         totalBytes - dbuf.consumedBytes(),
                         streamingCb);
    }
  } else {
    decodeStreamingImpl(requiredInsertCount, 0, dbuf, streamingCb);
  }
}

uint32_t QPACKDecoder::decodePrefix(HPACKDecodeBuffer& dbuf) {
  uint64_t requiredInsertCount;
  uint64_t wireRIC;
  uint64_t maxEntries = getMaxEntries(maxTableSize_);
  uint64_t fullRange = 2 * maxEntries;

  err_ = dbuf.decodeInteger(wireRIC);
  if (err_ != HPACK::DecodeError::NONE) {
    LOG(ERROR) << "Decode error decoding requiredInsertCount err_=" << err_;
    return 0;
  }
  if (wireRIC == 0) {
    requiredInsertCount = 0;
  } else if (maxEntries == 0) {
    LOG(ERROR) << "Encoder used dynamic table when not permitted, wireRIC="
               << wireRIC;
    err_ = HPACK::DecodeError::INVALID_INDEX;
    return 0;
  } else {
    uint64_t maxValue = table_.getInsertCount() + maxEntries;
    uint64_t maxWrapped = (maxValue / fullRange) * fullRange;
    requiredInsertCount = maxWrapped + wireRIC - 1;
    // If requiredInsertCount exceeds maxValue, the Encoder's value must have
    // wrapped one fewer time
    if (requiredInsertCount > maxValue) {
      if (wireRIC > fullRange || requiredInsertCount < fullRange) {
        LOG(ERROR) << "Decode error RIC out of range=" << wireRIC;
        err_ = HPACK::DecodeError::INVALID_INDEX;
        return 0;
      }
      requiredInsertCount -= fullRange;
    }
  }
  VLOG(5) << "Decoded requiredInsertCount=" << requiredInsertCount;
  uint64_t delta = 0;
  if (dbuf.empty()) {
    LOG(ERROR) << "Invalid prefix, no delta-base";
    err_ = HPACK::DecodeError::BUFFER_UNDERFLOW;
    return 0;
  }
  bool neg = dbuf.peek() & HPACK::Q_DELTA_BASE_NEG;
  err_ = dbuf.decodeInteger(HPACK::Q_DELTA_BASE.prefixLength, delta);
  if (err_ != HPACK::DecodeError::NONE) {
    LOG(ERROR) << "Decode error decoding delta base=" << err_;
    return 0;
  }
  if (neg) {
    // delta must be smaller than RIC
    if (delta >= requiredInsertCount) {
      LOG(ERROR) << "Received invalid delta=" << delta
                 << " requiredInsertCount=" << requiredInsertCount;
      err_ = HPACK::DecodeError::INVALID_INDEX;
      return 0;
    }
    // The largest table we support is 2^32 - 1 / 32 entries, so
    // requiredInsertCount (less any delta, etc) must be < 2^32.
    CHECK_LE(requiredInsertCount - delta - 1,
             std::numeric_limits<uint32_t>::max());
    baseIndex_ = requiredInsertCount - delta - 1;
  } else {
    // base must be < 2^32
    if (delta > std::numeric_limits<uint32_t>::max() ||
        requiredInsertCount >=
            uint64_t(std::numeric_limits<uint32_t>::max()) - delta) {
      LOG(ERROR) << "Invalid delta=" << delta
                 << " requiredInsertCount=" << requiredInsertCount;
      err_ = HPACK::DecodeError::INVALID_INDEX;
      return 0;
    }
    baseIndex_ = requiredInsertCount + delta;
  }
  VLOG(5) << "Decoded baseIndex_=" << baseIndex_;
  return requiredInsertCount;
}

void QPACKDecoder::decodeStreamingImpl(uint32_t requiredInsertCount,
                                       uint32_t consumed,
                                       HPACKDecodeBuffer& dbuf,
                                       HPACK::StreamingCallback* streamingCb) {
  uint32_t emittedSize = 0;

  while (!hasError() && !dbuf.empty()) {
    emittedSize += decodeHeaderQ(dbuf, streamingCb);
    if (emittedSize > maxUncompressed_) {
      LOG(ERROR) << "Exceeded uncompressed size limit of " << maxUncompressed_
                 << " bytes";
      err_ = HPACK::DecodeError::HEADERS_TOO_LARGE;
      break;
    }
    emittedSize += 2;
  }

  bool acknowledge = requiredInsertCount != 0;
  if (!hasError()) {
    // lastAcked_ is only read in encodeInsertCountInc, so all completed header
    // blocks must be call encodeHeaderAck BEFORE calling encodeInsertCountInc.
    lastAcked_ = std::max(lastAcked_, requiredInsertCount);
  }
  auto blockSize = consumed + dbuf.consumedBytes();
  auto compressedSize = pendingEncoderBytes_ + blockSize;
  pendingEncoderBytes_ = 0;
  completeDecode(HeaderCodec::Type::QPACK,
                 streamingCb,
                 compressedSize,
                 blockSize,
                 emittedSize,
                 acknowledge);
}

uint32_t QPACKDecoder::decodeHeaderQ(HPACKDecodeBuffer& dbuf,
                                     HPACK::StreamingCallback* streamingCb) {
  uint8_t byte = dbuf.peek();
  if (byte & HPACK::Q_INDEXED.code) {
    return decodeIndexedHeaderQ(
        dbuf, HPACK::Q_INDEXED.prefixLength, false, streamingCb, nullptr);
  } else if (byte & HPACK::Q_LITERAL_NAME_REF.code) {
    return decodeLiteralHeaderQ(dbuf,
                                false,
                                true,
                                HPACK::Q_LITERAL_NAME_REF.prefixLength,
                                false,
                                streamingCb);
  } else if (byte & HPACK::Q_LITERAL.code) {
    return decodeLiteralHeaderQ(
        dbuf, false, false, HPACK::Q_LITERAL.prefixLength, false, streamingCb);
  } else if (byte & HPACK::Q_INDEXED_POST.code) {
    return decodeIndexedHeaderQ(
        dbuf, HPACK::Q_INDEXED_POST.prefixLength, true, streamingCb, nullptr);
  } else { // Q_LITERAL_NAME_REF_POST
    return decodeLiteralHeaderQ(dbuf,
                                false,
                                true,
                                HPACK::Q_LITERAL_NAME_REF_POST.prefixLength,
                                true,
                                streamingCb);
  }
}

HPACK::DecodeError QPACKDecoder::decodeEncoderStream(
    std::unique_ptr<folly::IOBuf> buf) {
  ingress_.append(std::move(buf));
  Cursor cursor(ingress_.front());
  HPACKDecodeBuffer dbuf(cursor,
                         ingress_.chainLength(),
                         maxUncompressed_,
                         /* endOfBufferIsError=*/false);

  VLOG(6) << "Decoding control block";
  baseIndex_ = 0;
  err_ = HPACK::DecodeError::NONE;
  while (!hasError() && !dbuf.empty()) {
    decodeEncoderStreamInstruction(dbuf);
    if (err_ == HPACK::DecodeError::BUFFER_UNDERFLOW) {
      ingress_.trimStart(partial_.consumed);
      drainQueue();
      return HPACK::DecodeError::NONE;
    }
  }
  pendingEncoderBytes_ += dbuf.consumedBytes();
  ingress_.trimStart(dbuf.consumedBytes());
  if (hasError()) {
    return err_;
  } else {
    drainQueue();
    return HPACK::DecodeError::NONE;
  }
}

HPACK::DecodeError QPACKDecoder::encoderStreamEnd() {
  if (!ingress_.empty()) {
    err_ = HPACK::DecodeError::BUFFER_UNDERFLOW;
  }
  if (!queue_.empty()) {
    if (err_ != HPACK::DecodeError::NONE) {
      err_ = HPACK::DecodeError::ENCODER_STREAM_CLOSED;
    }
    errorQueue();
  }
  return err_;
}

void QPACKDecoder::decodeEncoderStreamInstruction(HPACKDecodeBuffer& dbuf) {
  uint8_t byte = dbuf.peek();
  partial_.consumed = dbuf.consumedBytes();
  if (partial_.state == Partial::VALUE ||
      byte & HPACK::Q_INSERT_NAME_REF.code) {
    // If partial state is VALUE, it might have been a NO_NAME_REF instruction,
    // but we've already parsed the name, so it doesn't matter
    decodeLiteralHeaderQ(dbuf,
                         true,
                         true,
                         HPACK::Q_INSERT_NAME_REF.prefixLength,
                         false,
                         nullptr);
  } else if (byte & HPACK::Q_INSERT_NO_NAME_REF.code) {
    decodeLiteralHeaderQ(dbuf,
                         true,
                         false,
                         HPACK::Q_INSERT_NO_NAME_REF.prefixLength,
                         false,
                         nullptr);
  } else if (byte & HPACK::Q_TABLE_SIZE_UPDATE.code) {
    handleTableSizeUpdate(dbuf, table_, true);
  } else { // must be Q_DUPLICATE=000
    headers_t emitted;
    decodeIndexedHeaderQ(
        dbuf, HPACK::Q_DUPLICATE.prefixLength, false, nullptr, &emitted);
    if (!hasError()) {
      CHECK(!emitted.empty());
      if (!table_.add(std::move(emitted[0]))) {
        // the only case is the header was > table capacity.  But how can we
        // duplicate such a header?
        LOG(DFATAL) << "Encoder duplicated a header larger than capacity";
        err_ = HPACK::DecodeError::INSERT_TOO_LARGE;
      } else {
        duplications_++;
      }
    }
  }
}

uint32_t QPACKDecoder::decodeLiteralHeaderQ(
    HPACKDecodeBuffer& dbuf,
    bool indexing,
    bool nameIndexed,
    uint8_t prefixLength,
    bool aboveBase,
    HPACK::StreamingCallback* streamingCb) {
  bool allowPartial = (streamingCb == nullptr);
  Partial localPartial;
  Partial* partial = (allowPartial) ? &partial_ : &localPartial;
  if (partial->state == Partial::NAME) {
    if (nameIndexed) {
      uint64_t nameIndex = 0;
      bool isStaticName = !aboveBase && (dbuf.peek() & (1 << prefixLength));
      err_ = dbuf.decodeInteger(prefixLength, nameIndex);
      if (allowPartial && err_ == HPACK::DecodeError::BUFFER_UNDERFLOW) {
        return 0;
      }
      if (err_ != HPACK::DecodeError::NONE) {
        LOG(ERROR) << "Decode error decoding index err_=" << err_;
        return 0;
      }
      nameIndex++;
      // validate the index
      if (!isValid(isStaticName, nameIndex, aboveBase)) {
        LOG(ERROR) << "Received invalid index=" << nameIndex;
        err_ = HPACK::DecodeError::INVALID_INDEX;
        return 0;
      }
      partial->header.name =
          getHeader(isStaticName, nameIndex, baseIndex_, aboveBase).name;
    } else {
      folly::fbstring headerName;
      err_ = dbuf.decodeLiteral(prefixLength, headerName);
      if (allowPartial && err_ == HPACK::DecodeError::BUFFER_UNDERFLOW) {
        return 0;
      }
      if (err_ != HPACK::DecodeError::NONE) {
        LOG(ERROR) << "Error decoding header name err_=" << err_;
        return 0;
      }
      partial->header.name = headerName;
    }
    partial->state = Partial::VALUE;
    partial->consumed = dbuf.consumedBytes();
  }
  // value
  err_ = dbuf.decodeLiteral(partial->header.value);
  if (allowPartial && err_ == HPACK::DecodeError::BUFFER_UNDERFLOW) {
    return 0;
  }
  if (err_ != HPACK::DecodeError::NONE) {
    LOG(ERROR) << "Error decoding header value name=" << partial->header.name
               << " err_=" << err_;
    return 0;
  }
  partial->state = Partial::NAME;

  uint32_t emittedSize = emit(partial->header, streamingCb, nullptr);

  if (indexing) {
    if (!table_.add(std::move(partial->header))) {
      // the only case is the header was > table capacity
      LOG(ERROR) << "Encoder inserted a header larger than capacity";
      err_ = HPACK::DecodeError::INSERT_TOO_LARGE;
    }
  }

  return emittedSize;
}

uint32_t QPACKDecoder::decodeIndexedHeaderQ(
    HPACKDecodeBuffer& dbuf,
    uint32_t prefixLength,
    bool aboveBase,
    HPACK::StreamingCallback* streamingCb,
    headers_t* emitted) {
  uint64_t index;
  bool isStatic = !aboveBase && (dbuf.peek() & (1 << prefixLength));
  err_ = dbuf.decodeInteger(prefixLength, index);
  if (err_ != HPACK::DecodeError::NONE) {
    if (streamingCb || err_ != HPACK::DecodeError::BUFFER_UNDERFLOW) {
      LOG(ERROR) << "Decode error decoding index err_=" << err_;
    }
    return 0;
  }
  CHECK_LT(index, std::numeric_limits<uint64_t>::max());
  index++;
  // validate the index
  if (index == 0 || !isValid(isStatic, index, aboveBase)) {
    LOG(ERROR) << "received invalid index: " << index;
    err_ = HPACK::DecodeError::INVALID_INDEX;
    return 0;
  }

  auto& header = getHeader(isStatic, index, baseIndex_, aboveBase);
  return emit(header, streamingCb, emitted);
}

bool QPACKDecoder::isValid(bool isStatic, uint64_t index, bool aboveBase) {
  if (index > std::numeric_limits<uint32_t>::max()) {
    return false;
  }
  if (isStatic) {
    return getStaticTable().isValid(index);
  } else {
    uint64_t baseIndex = baseIndex_;
    if (aboveBase) {
      baseIndex = baseIndex + index;
      if (baseIndex > std::numeric_limits<uint32_t>::max()) {
        return false;
      }
      index = 1;
    }
    return table_.isValid(index, baseIndex);
  }
}

std::unique_ptr<folly::IOBuf> QPACKDecoder::encodeInsertCountInc() {
  uint32_t toAck = table_.getInsertCount() - lastAcked_;
  if (toAck > 0) {
    VLOG(6) << "encodeInsertCountInc toAck=" << toAck;
    HPACKEncodeBuffer ackEncoder(kGrowth, false);
    ackEncoder.encodeInteger(toAck, HPACK::Q_INSERT_COUNT_INC);
    lastAcked_ = table_.getInsertCount();
    return ackEncoder.release();
  } else {
    return nullptr;
  }
}

std::unique_ptr<folly::IOBuf> QPACKDecoder::encodeHeaderAck(
    uint64_t streamId) const {
  HPACKEncodeBuffer ackEncoder(kGrowth, false);
  VLOG(6) << "encodeHeaderAck id=" << streamId;
  ackEncoder.encodeInteger(streamId, HPACK::Q_HEADER_ACK);
  return ackEncoder.release();
}

std::unique_ptr<folly::IOBuf> QPACKDecoder::encodeCancelStream(
    uint64_t streamId) {
  // Remove this stream from the queue
  VLOG(6) << "encodeCancelStream id=" << streamId;
  auto it = queue_.begin();
  while (it != queue_.end()) {
    if (it->second.streamID == streamId) {
      it = queue_.erase(it);
    } else {
      it++;
    }
  }
  HPACKEncodeBuffer ackEncoder(kGrowth, false);
  ackEncoder.encodeInteger(streamId, HPACK::Q_CANCEL_STREAM);
  return ackEncoder.release();
}

void QPACKDecoder::enqueueHeaderBlock(uint64_t streamID,
                                      uint32_t requiredInsertCount,
                                      uint32_t baseIndex,
                                      uint32_t consumed,
                                      std::unique_ptr<folly::IOBuf> block,
                                      size_t length,
                                      HPACK::StreamingCallback* streamingCb) {
  // TDOO: this queue is currently unbounded and has no timeouts
  CHECK_GT(requiredInsertCount, table_.getInsertCount());
  queue_.emplace(std::piecewise_construct,
                 std::forward_as_tuple(requiredInsertCount),
                 std::forward_as_tuple(streamID,
                                       baseIndex,
                                       length,
                                       consumed,
                                       std::move(block),
                                       streamingCb));
  holBlockCount_++;
  VLOG(5) << "queued block=" << requiredInsertCount << " len=" << length;
  queuedBytes_ += length;
}

bool QPACKDecoder::decodeBlock(uint32_t requiredInsertCount,
                               const PendingBlock& pending) {
  if (pending.length > 0) {
    VLOG(5) << "decodeBlock len=" << pending.length;
    folly::io::Cursor cursor(pending.block.get());
    HPACKDecodeBuffer dbuf(cursor, pending.length, maxUncompressed_);
    DCHECK_LE(pending.length, queuedBytes_);
    queuedBytes_ -= pending.length;
    baseIndex_ = pending.baseIndex;
    folly::DestructorCheck::Safety safety(*this);
    decodeStreamingImpl(
        requiredInsertCount, pending.consumed, dbuf, pending.cb);
    // The callback may destroy this, if so stop queue processing
    if (safety.destroyed()) {
      return true;
    }
  }
  return false;
}

void QPACKDecoder::drainQueue() {
  auto it = queue_.begin();
  while (!queue_.empty() && it->first <= table_.getInsertCount() &&
         !hasError()) {
    auto id = it->first;
    PendingBlock block = std::move(it->second);
    queue_.erase(it);
    if (decodeBlock(id, block)) {
      return;
    }
    it = queue_.begin();
  }
}

void QPACKDecoder::errorQueue() {
  // The callback may destroy this, if so stop queue processing
  folly::DestructorCheck::Safety safety(*this);
  while (!safety.destroyed() && !queue_.empty()) {
    auto it = queue_.begin();
    PendingBlock block = std::move(it->second);
    queue_.erase(it);
    block.cb->onDecodeError(HPACK::DecodeError::ENCODER_STREAM_CLOSED);
  }
}

} // namespace proxygen
