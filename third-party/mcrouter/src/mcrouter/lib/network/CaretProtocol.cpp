/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/network/CaretProtocol.h"

#include <folly/GroupVarint.h>
#include <folly/Range.h>
#include <folly/Varint.h>

#include "mcrouter/lib/network/CaretHeader.h"
#include "mcrouter/lib/network/ServerLoad.h"

namespace facebook {
namespace memcache {

namespace {

void resetAdditionalFields(CaretMessageInfo& info) {
  info.traceId = {0, 0};
  info.supportedCodecsFirstId = 0;
  info.supportedCodecsSize = 0;
  info.usedCodecId = 0;
  info.uncompressedBodySize = 0;
  info.dropProbability = 0;
  info.serverLoad = ServerLoad::zero();
}

size_t getNumAdditionalFields(const CaretMessageInfo& info) {
  size_t nAdditionalFields = 0;
  if (info.traceId.first != 0) {
    ++nAdditionalFields;
  }
  if (info.traceId.second != 0) {
    ++nAdditionalFields;
  }
  if (info.supportedCodecsFirstId != 0) {
    ++nAdditionalFields;
  }
  if (info.supportedCodecsSize != 0) {
    ++nAdditionalFields;
  }
  if (info.usedCodecId != 0) {
    ++nAdditionalFields;
  }
  if (info.uncompressedBodySize != 0) {
    ++nAdditionalFields;
  }
  if (info.dropProbability != 0) {
    ++nAdditionalFields;
  }
  if (!info.serverLoad.isZero()) {
    ++nAdditionalFields;
  }
  return nAdditionalFields;
}

/**
 * Serialize the additional field and return the number of bytes used to
 * serialize it.
 */
size_t serializeAdditionalFieldIfNonZero(
    uint8_t* destination,
    CaretAdditionalFieldType type,
    uint64_t value) {
  uint8_t* buf = destination;
  if (value > 0) {
    buf += folly::encodeVarint(static_cast<uint64_t>(type), buf);
    buf += folly::encodeVarint(value, buf);
  }
  return buf - destination;
}

size_t serializeAdditionalFields(
    uint8_t* destination,
    const CaretMessageInfo& info) {
  uint8_t* buf = destination;

  buf += serializeAdditionalFieldIfNonZero(
      buf, CaretAdditionalFieldType::TRACE_ID, info.traceId.first);
  buf += serializeAdditionalFieldIfNonZero(
      buf, CaretAdditionalFieldType::TRACE_NODE_ID, info.traceId.second);
  buf += serializeAdditionalFieldIfNonZero(
      buf,
      CaretAdditionalFieldType::SUPPORTED_CODECS_FIRST_ID,
      info.supportedCodecsFirstId);
  buf += serializeAdditionalFieldIfNonZero(
      buf,
      CaretAdditionalFieldType::SUPPORTED_CODECS_SIZE,
      info.supportedCodecsSize);
  buf += serializeAdditionalFieldIfNonZero(
      buf, CaretAdditionalFieldType::USED_CODEC_ID, info.usedCodecId);
  buf += serializeAdditionalFieldIfNonZero(
      buf,
      CaretAdditionalFieldType::UNCOMPRESSED_BODY_SIZE,
      info.uncompressedBodySize);
  buf += serializeAdditionalFieldIfNonZero(
      buf, CaretAdditionalFieldType::DROP_PROBABILITY, info.dropProbability);
  buf += serializeAdditionalFieldIfNonZero(
      buf, CaretAdditionalFieldType::SERVER_LOAD, info.serverLoad.raw());

  return buf - destination;
}

} // anonymous namespace

ParseStatus caretParseHeader(
    const uint8_t* buff,
    size_t nbuf,
    CaretMessageInfo& headerInfo) {
  /* we need the magic byte and the first byte of encoded header
     to determine if we have enough data in the buffer to get the
     entire header */
  if (nbuf < 2) {
    return ParseStatus::NotEnoughData;
  }

  if (buff[0] != kCaretMagicByte) {
    return ParseStatus::MessageParseError;
  }

  const char* buf = reinterpret_cast<const char*>(buff);
  size_t encodedLength = folly::GroupVarint32::encodedSize(buf + 1);

  if (nbuf < encodedLength + 1) {
    return ParseStatus::NotEnoughData;
  }

  uint32_t additionalFields;
  folly::GroupVarint32::decode_simple(
      buf + 1,
      &headerInfo.bodySize,
      &headerInfo.typeId,
      &headerInfo.reqId,
      &additionalFields);

  folly::StringPiece range(buf, nbuf);
  range.advance(encodedLength + 1);

  // Additional fields are sequence of (key,value) pairs
  resetAdditionalFields(headerInfo);
  for (uint32_t i = 0; i < additionalFields; i++) {
    size_t fieldType;
    if (auto maybeFieldType = folly::tryDecodeVarint(range)) {
      fieldType = *maybeFieldType;
    } else {
      return ParseStatus::NotEnoughData;
    }

    size_t fieldValue;
    if (auto maybeFieldValue = folly::tryDecodeVarint(range)) {
      fieldValue = *maybeFieldValue;
    } else {
      return ParseStatus::NotEnoughData;
    }

    if (fieldType >
        static_cast<uint64_t>(CaretAdditionalFieldType::SERVER_LOAD)) {
      // Additional Field Type not recognized, ignore.
      continue;
    }

    switch (static_cast<CaretAdditionalFieldType>(fieldType)) {
      case CaretAdditionalFieldType::TRACE_ID:
        headerInfo.traceId.first = fieldValue;
        break;
      case CaretAdditionalFieldType::TRACE_NODE_ID:
        headerInfo.traceId.second = fieldValue;
        break;
      case CaretAdditionalFieldType::SUPPORTED_CODECS_FIRST_ID:
        headerInfo.supportedCodecsFirstId = fieldValue;
        break;
      case CaretAdditionalFieldType::SUPPORTED_CODECS_SIZE:
        headerInfo.supportedCodecsSize = fieldValue;
        break;
      case CaretAdditionalFieldType::USED_CODEC_ID:
        headerInfo.usedCodecId = fieldValue;
        break;
      case CaretAdditionalFieldType::UNCOMPRESSED_BODY_SIZE:
        headerInfo.uncompressedBodySize = fieldValue;
        break;
      case CaretAdditionalFieldType::DROP_PROBABILITY:
        headerInfo.dropProbability = fieldValue;
        break;
      case CaretAdditionalFieldType::SERVER_LOAD:
        headerInfo.serverLoad = ServerLoad(fieldValue);
        break;
    }
  }

  headerInfo.headerSize = range.cbegin() - buf;

  return ParseStatus::Ok;
}

size_t caretPrepareHeader(const CaretMessageInfo& info, char* headerBuf) {
  // Header is at most kMaxHeaderLength without extra fields.

  uint32_t bodySize = info.bodySize;
  uint32_t typeId = info.typeId;
  uint32_t reqId = info.reqId;

  headerBuf[0] = kCaretMagicByte;

  // Header
  char* additionalFields = folly::GroupVarint32::encode(
      headerBuf + 1, bodySize, typeId, reqId, getNumAdditionalFields(info));

  // Additional fields
  additionalFields += serializeAdditionalFields(
      reinterpret_cast<uint8_t*>(additionalFields), info);

  return additionalFields - headerBuf;
}

} // namespace memcache
} // namespace facebook
