/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/HPACKDecoderBase.h>
#include <proxygen/lib/http/codec/compress/HeaderTable.h>

namespace proxygen {

uint32_t HPACKDecoderBase::emit(const HPACKHeader& header,
                                HPACK::StreamingCallback* streamingCb,
                                headers_t* emitted) {
  if (streamingCb) {
    streamingCb->onHeader(header.name, header.value);
  } else if (emitted) {
    // copying HPACKHeader
    emitted->emplace_back(header.name.get(), header.value);
  }
  return header.realBytes();
}

void HPACKDecoderBase::completeDecode(HeaderCodec::Type type,
                                      HPACK::StreamingCallback* streamingCb,
                                      uint32_t compressedSize,
                                      uint32_t compressedBlockSize,
                                      uint32_t emittedSize,
                                      bool acknowledge) {
  if (!streamingCb) {
    return;
  }
  if (err_ != HPACK::DecodeError::NONE) {
    if (streamingCb->stats) {
      if (err_ == HPACK::DecodeError::HEADERS_TOO_LARGE ||
          err_ == HPACK::DecodeError::LITERAL_TOO_LARGE) {
        streamingCb->stats->recordDecodeTooLarge(type);
      } else {
        streamingCb->stats->recordDecodeError(type);
      }
    }
    streamingCb->onDecodeError(err_);
  } else {
    HTTPHeaderSize decodedSize;
    decodedSize.compressed = compressedSize;
    decodedSize.compressedBlock = compressedBlockSize,
    decodedSize.uncompressed = emittedSize;
    if (streamingCb->stats) {
      streamingCb->stats->recordDecode(type, decodedSize);
    }
    streamingCb->onHeadersComplete(decodedSize, acknowledge);
  }
}

void HPACKDecoderBase::setHeaderTableMaxSize(HeaderTable& table,
                                             uint32_t maxSize) {
  maxTableSize_ = maxSize;
  if (maxTableSize_ < table.capacity()) {
    CHECK(table.setCapacity(maxTableSize_));
  }
}

void HPACKDecoderBase::handleTableSizeUpdate(HPACKDecodeBuffer& dbuf,
                                             HeaderTable& table,
                                             bool isQpack) {
  uint64_t arg = 0;
  err_ = dbuf.decodeInteger(HPACK::TABLE_SIZE_UPDATE.prefixLength, arg);
  if (err_ != HPACK::DecodeError::NONE) {
    if (!isQpack || err_ != HPACK::DecodeError::BUFFER_UNDERFLOW) {
      LOG(ERROR) << "Decode error decoding maxSize err_=" << err_;
    }
    return;
  }

  if (arg > maxTableSize_) {
    LOG(ERROR) << "Tried to increase size of the header table to " << arg
               << " maxTableSize_=" << maxTableSize_;
    err_ = HPACK::DecodeError::INVALID_TABLE_SIZE;
    return;
  }
  VLOG(5) << "Received table size update, new size=" << arg;
  table.setCapacity(arg);
}

} // namespace proxygen
