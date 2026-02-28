/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/HPACKDecoder.h>

#include <proxygen/lib/http/codec/compress/HeaderCodec.h>

using folly::io::Cursor;

namespace proxygen {

void HPACKDecoder::decodeStreaming(Cursor& cursor,
                                   uint32_t totalBytes,
                                   HPACK::StreamingCallback* streamingCb) {
  HPACKDecodeBuffer dbuf(cursor, totalBytes, maxUncompressed_);
  uint32_t emittedSize = 0;

  while (!hasError() && !dbuf.empty()) {
    emittedSize += decodeHeader(dbuf, streamingCb, nullptr);

    if (emittedSize > maxUncompressed_) {
      LOG(ERROR) << "exceeded uncompressed size limit of " << maxUncompressed_
                 << " bytes";
      err_ = HPACK::DecodeError::HEADERS_TOO_LARGE;
      break;
    }
    emittedSize += 2;
  }
  auto compressedSize = dbuf.consumedBytes();
  completeDecode(HeaderCodec::Type::HPACK,
                 streamingCb,
                 compressedSize,
                 compressedSize,
                 emittedSize);
}

uint32_t HPACKDecoder::decodeLiteralHeader(
    HPACKDecodeBuffer& dbuf,
    HPACK::StreamingCallback* streamingCb,
    headers_t* emitted) {
  uint8_t byte = dbuf.peek();
  bool indexing = byte & HPACK::LITERAL_INC_INDEX.code;
  HPACKHeader header;
  uint8_t indexMask = 0x3F; // 0011 1111
  uint8_t length = HPACK::LITERAL_INC_INDEX.prefixLength;
  if (!indexing) {
    // bool neverIndex = byte & HPACK::LITERAL_NEV_INDEX.code;
    // TODO: we need to emit this flag with the headers
    indexMask = 0x0F; // 0000 1111
    length = HPACK::LITERAL.prefixLength;
  }
  if (byte & indexMask) {
    uint64_t index;
    err_ = dbuf.decodeInteger(length, index);
    if (err_ != HPACK::DecodeError::NONE) {
      LOG(ERROR) << "Decode error decoding index err_=" << err_;
      return 0;
    }
    // validate the index
    if (!isValid(index)) {
      LOG(ERROR) << "received invalid index: " << index;
      err_ = HPACK::DecodeError::INVALID_INDEX;
      return 0;
    }
    header.name = getHeader(index).name;
  } else {
    // skip current byte
    dbuf.next();
    folly::fbstring headerName;
    err_ = dbuf.decodeLiteral(headerName);
    header.name = headerName;
    if (err_ != HPACK::DecodeError::NONE) {
      LOG(ERROR) << "Error decoding header name err_=" << err_;
      return 0;
    }
  }
  // value
  err_ = dbuf.decodeLiteral(header.value);
  if (err_ != HPACK::DecodeError::NONE) {
    LOG(ERROR) << "Error decoding header value name=" << header.name
               << " err_=" << err_;
    return 0;
  }

  uint32_t emittedSize = emit(header, streamingCb, emitted);

  if (indexing) {
    auto headerBytes = header.bytes();
    if (!table_.add(std::move(header))) {
      // The only way add can return false is clearing the table with a large
      // entry.  Any other failure would result in compression contexts out of
      // sync.
      CHECK_GT(headerBytes, table_.capacity());
    }
  }

  return emittedSize;
}

uint32_t HPACKDecoder::decodeIndexedHeader(
    HPACKDecodeBuffer& dbuf,
    HPACK::StreamingCallback* streamingCb,
    headers_t* emitted) {
  uint64_t index;
  err_ = dbuf.decodeInteger(HPACK::INDEX_REF.prefixLength, index);
  if (err_ != HPACK::DecodeError::NONE) {
    LOG(ERROR) << "Decode error decoding index err_=" << err_;
    return 0;
  }
  // validate the index
  if (index == 0 || !isValid(index)) {
    LOG(ERROR) << "received invalid index: " << index;
    err_ = HPACK::DecodeError::INVALID_INDEX;
    return 0;
  }

  const auto& header = getHeader(index);
  return emit(header, streamingCb, emitted);
}

bool HPACKDecoder::isValid(uint32_t index) {
  if (isStatic(index)) {
    return getStaticTable().isValid(globalToStaticIndex(index));
  } else {
    return table_.isValid(globalToDynamicIndex(index));
  }
}

uint32_t HPACKDecoder::decodeHeader(HPACKDecodeBuffer& dbuf,
                                    HPACK::StreamingCallback* streamingCb,
                                    headers_t* emitted) {
  uint8_t byte = dbuf.peek();
  if (byte & HPACK::INDEX_REF.code) {
    return decodeIndexedHeader(dbuf, streamingCb, emitted);
  } else if (byte & HPACK::LITERAL_INC_INDEX.code) {
    // else it's fine, fall through to decodeLiteralHeader
  } else if (byte & HPACK::TABLE_SIZE_UPDATE.code) {
    handleTableSizeUpdate(dbuf, table_);
    return 0;
  } // else LITERAL
  // LITERAL_NO_INDEXING or LITERAL_INCR_INDEXING
  return decodeLiteralHeader(dbuf, streamingCb, emitted);
}

} // namespace proxygen
