/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HTTP2Framer.h>

#include <folly/tracing/ScopedTraceSection.h>

using namespace folly::io;
using namespace folly;

namespace proxygen { namespace http2 {

const uint8_t kMinExperimentalFrameType = 0xf0;
const Padding kNoPadding = folly::none;
const PriorityUpdate DefaultPriority{0, false, 15};

namespace {

const uint32_t kLengthMask = 0x00ffffff;
const uint32_t kUint31Mask = 0x7fffffff;

static const uint64_t kZeroPad[32] = {0};

static const bool kStrictPadding = true;

static_assert(sizeof(kZeroPad) == 256, "bad zero padding");

void writePriorityBody(QueueAppender& appender,
                       uint32_t streamDependency,
                       bool exclusive,
                       uint8_t weight) {
  DCHECK_EQ(0, ~kUint31Mask & streamDependency);

  if (exclusive) {
    streamDependency |= ~kUint31Mask;
  }

  appender.writeBE<uint32_t>(streamDependency);
  appender.writeBE<uint8_t>(weight);
}

void writePriorityBody(uint8_t* buf,
                       uint32_t streamDependency,
                       bool exclusive,
                       uint8_t weight) {
  DCHECK_EQ(0, ~kUint31Mask & streamDependency);

  if (exclusive) {
    streamDependency |= ~kUint31Mask;
  }
  streamDependency = htonl(streamDependency);
  memcpy(buf, &streamDependency, sizeof(streamDependency));
  buf += sizeof(streamDependency);
  *buf = weight;
}

void writePadding(IOBufQueue& queue, folly::Optional<uint8_t> size) {
  if (size && *size > 0) {
    auto out = queue.preallocate(*size, *size);
    memset(out.first, 0, *size);
    queue.postallocate(*size);
  }
}

/**
 * Generate just the common frame header. This includes the padding length
 * bits that sometimes come right after the frame header. Returns the
 * length field written to the common frame header.
 */

size_t computeLengthAndType(uint32_t length,
                            FrameType type,
                            uint8_t& flags,
                            uint32_t stream,
                            folly::Optional<uint8_t> padding,
                            folly::Optional<PriorityUpdate> priority,
                            size_t& headerSize) {
  // the acceptable length is now conditional based on state :(
  DCHECK_EQ(0, ~kLengthMask & length);
  DCHECK_EQ(0, ~kUint31Mask & stream);

  if (priority) {
    if (FrameType::HEADERS == type || FrameType::EX_HEADERS == type) {
      DCHECK(flags & PRIORITY);
      length += kFramePrioritySize;
    } else {
      DCHECK(FrameType::PRIORITY == type) << "priority is unexpected";
    }
    headerSize += kFramePrioritySize;
    DCHECK_EQ(0, ~kLengthMask & length);
    DCHECK_NE(priority->streamDependency, stream) << "Circular dependency";
  }

  // Add or remove padding flags
  if (padding) {
    flags |= PADDED;
    DCHECK(FrameType::HEADERS == type || FrameType::EX_HEADERS == type ||
           FrameType::DATA == type || FrameType::PUSH_PROMISE == type);
    length += *padding + 1;
    headerSize += 1;
  } else {
    flags &= ~PADDED;
  }

  DCHECK_EQ(0, ~kLengthMask & length);
  DCHECK_EQ(true, isValidFrameType(type));
  return ((kLengthMask & length) << 8) | static_cast<uint8_t>(type);
}

// There are two versions of writeFrameHeader.  One takes an IOBufQueue and
// uses a QueueAppender.  The other takes a raw buffer.

size_t writeFrameHeader(IOBufQueue& queue,
                        uint32_t length,
                        FrameType type,
                        uint8_t flags,
                        uint32_t stream,
                        folly::Optional<uint8_t> padding,
                        folly::Optional<PriorityUpdate> priority,
                        std::unique_ptr<IOBuf> payload,
                        bool reuseIOBufHeadroom = true) noexcept {
  size_t headerSize = kFrameHeaderSize;
  uint32_t lengthAndType = computeLengthAndType(
      length, type, flags, stream, padding, priority, headerSize);

  uint64_t payloadLength = 0;
  if (reuseIOBufHeadroom && payload && !payload->isSharedOne() &&
      payload->headroom() >= headerSize && queue.tailroom() < headerSize) {
    // Use the headroom in payload for the frame header.
    // Make it appear that the payload IOBuf is empty and retreat so
    // appender can access the headroom
    payloadLength = payload->length();
    payload->trimEnd(payloadLength);
    payload->retreat(headerSize);
    auto tail = payload->pop();
    queue.append(std::move(payload));
    payload = std::move(tail);
  }
  QueueAppender appender(&queue, headerSize);
  appender.writeBE<uint32_t>(lengthAndType);
  appender.writeBE<uint8_t>(flags);
  appender.writeBE<uint32_t>(kUint31Mask & stream);

  if (padding) {
    appender.writeBE<uint8_t>(*padding);
  }
  if (priority) {
    CHECK_LE(priority->streamDependency, std::numeric_limits<uint32_t>::max());
    writePriorityBody(appender,
                      (uint32_t)priority->streamDependency,
                      priority->exclusive,
                      priority->weight);
  }
  if (payloadLength) {
    queue.postallocate(payloadLength);
  }
  queue.append(std::move(payload));

  return length;
}

size_t writeFrameHeader(uint8_t* buf,
                        size_t bufLen,
                        uint32_t length,
                        FrameType type,
                        uint8_t flags,
                        uint32_t stream,
                        folly::Optional<uint8_t> padding,
                        folly::Optional<PriorityUpdate> priority) noexcept {
  size_t headerSize = kFrameHeaderSize;
  uint32_t lengthAndType = computeLengthAndType(
      length, type, flags, stream, padding, priority, headerSize);

  CHECK_GE(bufLen, headerSize);
  lengthAndType = htonl(lengthAndType);
  memcpy(buf, &lengthAndType, sizeof(lengthAndType));
  buf += sizeof(lengthAndType);
  *buf = flags;
  buf++;
  stream &= kUint31Mask;
  stream = htonl(stream);
  memcpy(buf, &stream, sizeof(stream));
  buf += sizeof(stream);
  bufLen -= kFrameHeaderSize;

  if (padding) {
    CHECK_GE(bufLen, 1);
    *buf = *padding;
    buf++;
    bufLen--;
  }
  if (priority) {
    CHECK_GE(bufLen, kFramePrioritySize);
    CHECK_LE(priority->streamDependency, std::numeric_limits<uint32_t>::max());
    writePriorityBody(buf,
                      (uint32_t)priority->streamDependency,
                      priority->exclusive,
                      priority->weight);
  }
  return length;
}

uint32_t parseUint31(Cursor& cursor) {
  // MUST ignore the 1 bit before the stream-id
  return kUint31Mask & cursor.readBE<uint32_t>();
}

ErrorCode parseErrorCode(Cursor& cursor, ErrorCode& outCode) {
  auto code = cursor.readBE<uint32_t>();
  if (code > kMaxErrorCode) {
    return ErrorCode::PROTOCOL_ERROR;
  }
  outCode = ErrorCode(code);
  return ErrorCode::NO_ERROR;
}

PriorityUpdate parsePriorityCommon(Cursor& cursor) {
  PriorityUpdate priority;
  uint32_t streamAndExclusive = cursor.readBE<uint32_t>();
  priority.weight = cursor.readBE<uint8_t>();
  priority.exclusive = ~kUint31Mask & streamAndExclusive;
  priority.streamDependency = kUint31Mask & streamAndExclusive;
  return priority;
}

/**
 * Given the flags for a frame and the cursor pointing at the top of the
 * frame-specific section (after the common header), return the number of
 * bytes to skip at the end of the frame. Caller must ensure there is at
 * least 1 bytes in the cursor.
 *
 * @param cursor The cursor to pull data from
 * @param header The frame header for the frame being parsed.
 * @param padding The out parameter that will return the number of padding
 *                bytes at the end of the frame.
 * @return Nothing if success. The connection error code if failure.
 */
ErrorCode parsePadding(Cursor& cursor,
                       const FrameHeader& header,
                       uint8_t& padding,
                       uint32_t& lefttoparse) noexcept {
  DCHECK(header.type == FrameType::DATA || header.type == FrameType::HEADERS ||
         header.type == FrameType::EX_HEADERS ||
         header.type == FrameType::PUSH_PROMISE);
  lefttoparse = header.length;
  if (frameHasPadding(header)) {
    if (lefttoparse < 1) {
      return ErrorCode::FRAME_SIZE_ERROR;
    }
    lefttoparse -= 1;
    padding = cursor.readBE<uint8_t>();
  } else {
    padding = 0;
  }

  if (lefttoparse < padding) {
    return ErrorCode::PROTOCOL_ERROR;
  } else {
    lefttoparse -= padding;
    return ErrorCode::NO_ERROR;
  }
}

ErrorCode skipPadding(Cursor& cursor, uint8_t length, bool verify) {
  if (verify) {
    while (length > 0) {
      auto cur = cursor.peek();
      uint8_t toCmp = std::min<size_t>(cur.second, length);
      if (memcmp(cur.first, kZeroPad, toCmp)) {
        return ErrorCode::PROTOCOL_ERROR;
      }
      cursor.skip(toCmp);
      length -= toCmp;
    }
  } else {
    cursor.skip(length);
  }
  return ErrorCode::NO_ERROR;
}

} // anonymous namespace

bool isValidFrameType(FrameType type) {
  auto val = static_cast<uint8_t>(type);
  if (val < kMinExperimentalFrameType) {
    return val <= static_cast<uint8_t>(FrameType::ALTSVC);
  } else {
    switch (type) {
      case FrameType::EX_HEADERS:
        // Include the frame types added into FrameType enum for secondary
        // authentication.
      case FrameType::CERTIFICATE_REQUEST:
      case FrameType::CERTIFICATE:
        return true;
      default:
        return false;
    }
  }
}

bool frameAffectsCompression(FrameType t) {
  return t == FrameType::HEADERS || t == FrameType::PUSH_PROMISE ||
         t == FrameType::CONTINUATION;
}

bool frameHasPadding(const FrameHeader& header) {
  return header.flags & PADDED;
}

//// Parsing ////

ErrorCode parseFrameHeader(Cursor& cursor, FrameHeader& header) noexcept {
  FOLLY_SCOPED_TRACE_SECTION("HTTP2Framer - parseFrameHeader");
  DCHECK_LE(kFrameHeaderSize, cursor.totalLength());

  // MUST ignore the 2 bits before the length
  uint32_t lengthAndType = cursor.readBE<uint32_t>();
  header.length = kLengthMask & (lengthAndType >> 8);
  uint8_t type = lengthAndType & 0xff;
  header.type = FrameType(type);
  header.flags = cursor.readBE<uint8_t>();
  header.stream = parseUint31(cursor);
  return ErrorCode::NO_ERROR;
}

ErrorCode parseData(Cursor& cursor,
                    const FrameHeader& header,
                    std::unique_ptr<IOBuf>& outBuf,
                    uint16_t& outPadding) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.stream == 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }

  uint8_t padding;
  uint32_t lefttoparse;
  const auto err = parsePadding(cursor, header, padding, lefttoparse);
  RETURN_IF_ERROR(err);
  // outPadding is the total number of flow-controlled pad bytes, which
  // includes the length byte, if present.
  outPadding = padding + ((frameHasPadding(header)) ? 1 : 0);
  cursor.clone(outBuf, lefttoparse);
  return skipPadding(cursor, padding, kStrictPadding);
}

ErrorCode parseDataBegin(Cursor& cursor,
                         const FrameHeader& header,
                         size_t& /*parsed*/,
                         uint16_t& outPadding) noexcept {
  uint8_t padding;
  uint32_t lefttoparse;
  const auto err = http2::parsePadding(cursor, header, padding, lefttoparse);
  RETURN_IF_ERROR(err);
  // outPadding is the total number of flow-controlled pad bytes, which
  // includes the length byte, if present.
  outPadding = padding + ((frameHasPadding(header)) ? 1 : 0);
  return ErrorCode::NO_ERROR;
}

ErrorCode parseDataEnd(Cursor& cursor,
                       const size_t bufLen,
                       const size_t pendingDataFramePaddingBytes,
                       size_t& toSkip) noexcept {
  toSkip = std::min(pendingDataFramePaddingBytes, bufLen);
  return skipPadding(cursor, toSkip, kStrictPadding);
}

ErrorCode parseHeaders(Cursor& cursor,
                       const FrameHeader& header,
                       folly::Optional<PriorityUpdate>& outPriority,
                       std::unique_ptr<IOBuf>& outBuf) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.stream == 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }
  uint8_t padding;
  uint32_t lefttoparse;
  auto err = parsePadding(cursor, header, padding, lefttoparse);
  RETURN_IF_ERROR(err);
  if (header.flags & PRIORITY) {
    if (lefttoparse < kFramePrioritySize) {
      return ErrorCode::FRAME_SIZE_ERROR;
    }
    outPriority = parsePriorityCommon(cursor);
    lefttoparse -= kFramePrioritySize;
  } else {
    outPriority = folly::none;
  }
  cursor.clone(outBuf, lefttoparse);
  return skipPadding(cursor, padding, kStrictPadding);
}

ErrorCode parseExHeaders(Cursor& cursor,
                         const FrameHeader& header,
                         HTTPCodec::ExAttributes& outExAttributes,
                         folly::Optional<PriorityUpdate>& outPriority,
                         std::unique_ptr<IOBuf>& outBuf) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.stream == 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }

  uint8_t padding;
  uint32_t lefttoparse;
  auto err = parsePadding(cursor, header, padding, lefttoparse);
  RETURN_IF_ERROR(err);

  // the regular HEADERS frame starts from here
  if (header.flags & PRIORITY) {
    if (lefttoparse < kFramePrioritySize) {
      return ErrorCode::FRAME_SIZE_ERROR;
    }
    outPriority = parsePriorityCommon(cursor);
    lefttoparse -= kFramePrioritySize;
  } else {
    outPriority = folly::none;
  }
  outExAttributes.unidirectional = header.flags & UNIDIRECTIONAL;

  if (lefttoparse < kFrameStreamIDSize) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  outExAttributes.controlStream = parseUint31(cursor);
  lefttoparse -= kFrameStreamIDSize;
  if (!(outExAttributes.controlStream & 0x1)) {
    // control stream ID should be odd because it is initiated by client
    return ErrorCode::PROTOCOL_ERROR;
  }

  cursor.clone(outBuf, lefttoparse);
  return skipPadding(cursor, padding, kStrictPadding);
}

ErrorCode parsePriority(Cursor& cursor,
                        const FrameHeader& header,
                        PriorityUpdate& outPriority) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.length != kFramePrioritySize) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  if (header.stream == 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }
  outPriority = parsePriorityCommon(cursor);
  return ErrorCode::NO_ERROR;
}

ErrorCode parseRstStream(Cursor& cursor,
                         const FrameHeader& header,
                         ErrorCode& outCode) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.length != kFrameRstStreamSize) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  if (header.stream == 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }
  return parseErrorCode(cursor, outCode);
}

ErrorCode parseSettings(Cursor& cursor,
                        const FrameHeader& header,
                        std::deque<SettingPair>& settings) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.stream != 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }
  if (header.flags & ACK) {
    if (header.length != 0) {
      return ErrorCode::FRAME_SIZE_ERROR;
    }
    return ErrorCode::NO_ERROR;
  }

  if (header.length % 6 != 0) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  for (auto length = header.length; length > 0; length -= 6) {
    uint16_t id = cursor.readBE<uint16_t>();
    uint32_t val = cursor.readBE<uint32_t>();
    settings.push_back(std::make_pair(SettingsId(id), val));
  }
  return ErrorCode::NO_ERROR;
}

ErrorCode parsePushPromise(Cursor& cursor,
                           const FrameHeader& header,
                           uint32_t& outPromisedStream,
                           std::unique_ptr<IOBuf>& outBuf) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.stream == 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }

  uint8_t padding;
  uint32_t lefttoparse;
  auto err = parsePadding(cursor, header, padding, lefttoparse);
  RETURN_IF_ERROR(err);
  if (lefttoparse < kFramePushPromiseSize) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  lefttoparse -= kFramePushPromiseSize;
  outPromisedStream = parseUint31(cursor);
  if (outPromisedStream == 0 || outPromisedStream & 0x1) {
    // client MUST reserve an even stream id greater than 0
    return ErrorCode::PROTOCOL_ERROR;
  }
  if (lefttoparse < padding) {
    return ErrorCode::PROTOCOL_ERROR;
  }
  cursor.clone(outBuf, lefttoparse);
  return skipPadding(cursor, padding, kStrictPadding);
}

ErrorCode parsePing(Cursor& cursor,
                    const FrameHeader& header,
                    uint64_t& outOpaqueData) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());

  if (header.length != kFramePingSize) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  if (header.stream != 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }

  cursor.pull(&outOpaqueData, sizeof(outOpaqueData));
  return ErrorCode::NO_ERROR;
}

ErrorCode parseGoaway(Cursor& cursor,
                      const FrameHeader& header,
                      uint32_t& outLastStreamID,
                      ErrorCode& outCode,
                      std::unique_ptr<IOBuf>& outDebugData) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.length < kFrameGoawaySize) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  if (header.stream != 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }
  outLastStreamID = parseUint31(cursor);
  auto err = parseErrorCode(cursor, outCode);
  RETURN_IF_ERROR(err);
  auto length = header.length;
  length -= kFrameGoawaySize;
  if (length > 0) {
    cursor.clone(outDebugData, length);
  }
  return ErrorCode::NO_ERROR;
}

ErrorCode parseWindowUpdate(Cursor& cursor,
                            const FrameHeader& header,
                            uint32_t& outAmount) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.length != kFrameWindowUpdateSize) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  outAmount = parseUint31(cursor);
  return ErrorCode::NO_ERROR;
}

ErrorCode parseContinuation(Cursor& cursor,
                            const FrameHeader& header,
                            std::unique_ptr<IOBuf>& outBuf) noexcept {
  DCHECK(header.type == FrameType::CONTINUATION);
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.stream == 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }
  cursor.clone(outBuf, header.length);
  return ErrorCode::NO_ERROR;
}

ErrorCode parseAltSvc(Cursor& cursor,
                      const FrameHeader& header,
                      uint32_t& outMaxAge,
                      uint32_t& outPort,
                      std::string& outProtocol,
                      std::string& outHost,
                      std::string& outOrigin) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.length < kFrameAltSvcSizeBase) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  std::unique_ptr<IOBuf> tmpBuf;

  outMaxAge = cursor.readBE<uint32_t>();
  outPort = cursor.readBE<uint16_t>();
  const auto protoLen = cursor.readBE<uint8_t>();
  if (header.length < kFrameAltSvcSizeBase + protoLen) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  outProtocol = cursor.readFixedString(protoLen);
  const auto hostLen = cursor.readBE<uint8_t>();
  if (header.length < kFrameAltSvcSizeBase + protoLen + hostLen) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  outHost = cursor.readFixedString(hostLen);
  const auto originLen =
      (header.length - kFrameAltSvcSizeBase - protoLen - hostLen);
  outOrigin = cursor.readFixedString(originLen);

  return ErrorCode::NO_ERROR;
}

ErrorCode parseCertificateRequest(
    folly::io::Cursor& cursor,
    const FrameHeader& header,
    uint16_t& outRequestId,
    std::unique_ptr<folly::IOBuf>& outAuthRequest) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.length < kFrameCertificateRequestSizeBase) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  if (header.stream != 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }
  outRequestId = cursor.readBE<uint16_t>();
  auto length = header.length;
  length -= kFrameCertificateRequestSizeBase;
  if (length > 0) {
    cursor.clone(outAuthRequest, length);
  }
  return ErrorCode::NO_ERROR;
}

ErrorCode parseCertificate(
    folly::io::Cursor& cursor,
    const FrameHeader& header,
    uint16_t& outCertId,
    std::unique_ptr<folly::IOBuf>& outAuthenticator) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  if (header.length < kFrameCertificateSizeBase) {
    return ErrorCode::FRAME_SIZE_ERROR;
  }
  if (header.stream != 0) {
    return ErrorCode::PROTOCOL_ERROR;
  }
  outCertId = cursor.readBE<uint16_t>();
  auto length = header.length;
  length -= kFrameCertificateSizeBase;
  if (length > 0) {
    cursor.clone(outAuthenticator, length);
  }
  return ErrorCode::NO_ERROR;
}

//// Egress ////

size_t writeData(IOBufQueue& queue,
                 std::unique_ptr<IOBuf> data,
                 uint32_t stream,
                 folly::Optional<uint8_t> padding,
                 bool endStream,
                 bool reuseIOBufHeadroom) noexcept {
  DCHECK_NE(0, stream);
  uint8_t flags = 0;
  if (endStream) {
    flags |= END_STREAM;
  }
  const uint64_t dataLen = data ? data->computeChainDataLength() : 0;
  // Caller must not exceed peer setting for MAX_FRAME_SIZE
  // TODO: look into using headroom from data to hold the frame header
  const auto frameLen = writeFrameHeader(queue,
                                         dataLen,
                                         FrameType::DATA,
                                         flags,
                                         stream,
                                         padding,
                                         folly::none,
                                         std::move(data),
                                         reuseIOBufHeadroom);
  writePadding(queue, padding);
  return kFrameHeaderSize + frameLen;
}

uint8_t calculatePreHeaderBlockSize(bool hasAssocStream,
                                    bool hasExAttributes,
                                    bool hasPriority,
                                    bool hasPadding) {
  uint8_t headerSize =
      http2::kFrameHeaderSize +
      ((hasAssocStream || hasExAttributes) ? sizeof(uint32_t) : 0);
  if (hasPriority && !hasAssocStream) {
    headerSize += http2::kFramePrioritySize;
  }
  if (hasPadding) {
    headerSize += 1;
  }
  return headerSize;
}

size_t writeHeaders(uint8_t* header,
                    size_t headerLen,
                    IOBufQueue& queue,
                    size_t headersLen,
                    uint32_t stream,
                    folly::Optional<PriorityUpdate> priority,
                    folly::Optional<uint8_t> padding,
                    bool endStream,
                    bool endHeaders) noexcept {
  DCHECK_NE(0, stream);
  uint32_t flags = 0;
  if (priority) {
    flags |= PRIORITY;
  }
  if (endStream) {
    flags |= END_STREAM;
  }
  if (endHeaders) {
    flags |= END_HEADERS;
  }
  // padding flags handled directly inside writeFrameHeader()
  const auto frameLen = writeFrameHeader(header,
                                         headerLen,
                                         headersLen,
                                         FrameType::HEADERS,
                                         flags,
                                         stream,
                                         padding,
                                         priority);
  writePadding(queue, padding);
  return kFrameHeaderSize + frameLen;
}

size_t writeExHeaders(uint8_t* header,
                      size_t headerLen,
                      IOBufQueue& queue,
                      size_t headersLen,
                      uint32_t stream,
                      const HTTPCodec::ExAttributes& exAttributes,
                      const folly::Optional<PriorityUpdate>& priority,
                      const folly::Optional<uint8_t>& padding,
                      bool endStream,
                      bool endHeaders) noexcept {
  DCHECK_NE(0, stream);
  DCHECK_NE(0, exAttributes.controlStream);
  DCHECK_EQ(0, ~kUint31Mask & stream);
  DCHECK_EQ(0, ~kUint31Mask & exAttributes.controlStream);
  DCHECK(0x1 & exAttributes.controlStream)
      << "controlStream should be initiated by client";

  uint32_t flags = 0;
  if (priority) {
    flags |= PRIORITY;
  }
  if (endStream) {
    flags |= END_STREAM;
  }
  if (endHeaders) {
    flags |= END_HEADERS;
  }
  if (exAttributes.unidirectional) {
    flags |= UNIDIRECTIONAL;
  }

  const auto frameLen = writeFrameHeader(header,
                                         headerLen,
                                         headersLen + kFrameStreamIDSize,
                                         FrameType::EX_HEADERS,
                                         flags,
                                         stream,
                                         padding,
                                         priority);
  uint8_t* csPtr = header + kFrameHeaderSize + ((padding) ? 1 : 0) +
                   ((priority) ? kFramePrioritySize : 0);
  auto controlStream = htonl(exAttributes.controlStream);
  memcpy(csPtr, &controlStream, sizeof(controlStream));
  QueueAppender appender(&queue, frameLen);
  writePadding(queue, padding);
  return kFrameHeaderSize + frameLen;
}

size_t writePriority(IOBufQueue& queue,
                     uint32_t stream,
                     PriorityUpdate priority) noexcept {
  DCHECK_NE(0, stream);
  const auto frameLen = writeFrameHeader(queue,
                                         kFramePrioritySize,
                                         FrameType::PRIORITY,
                                         0,
                                         stream,
                                         kNoPadding,
                                         priority,
                                         nullptr);
  return kFrameHeaderSize + frameLen;
}

size_t writeRstStream(IOBufQueue& queue,
                      uint32_t stream,
                      ErrorCode errorCode) noexcept {
  DCHECK_NE(0, stream);
  const auto frameLen = writeFrameHeader(queue,
                                         kFrameRstStreamSize,
                                         FrameType::RST_STREAM,
                                         0,
                                         stream,
                                         kNoPadding,
                                         folly::none,
                                         nullptr);
  QueueAppender appender(&queue, frameLen);
  appender.writeBE<uint32_t>(static_cast<uint32_t>(errorCode));
  return kFrameHeaderSize + frameLen;
}

size_t writeSettings(IOBufQueue& queue,
                     const std::deque<SettingPair>& settings) {
  const auto settingsSize = settings.size() * 6;
  const auto frameLen = writeFrameHeader(queue,
                                         settingsSize,
                                         FrameType::SETTINGS,
                                         0,
                                         0,
                                         kNoPadding,
                                         folly::none,
                                         nullptr);
  QueueAppender appender(&queue, settingsSize);
  for (const auto& setting : settings) {
    DCHECK_LE(static_cast<uint32_t>(setting.first),
              std::numeric_limits<uint16_t>::max());
    appender.writeBE<uint16_t>(static_cast<uint16_t>(setting.first));
    appender.writeBE<uint32_t>(setting.second);
  }
  return kFrameHeaderSize + frameLen;
}

size_t writeSettingsAck(IOBufQueue& queue) {
  writeFrameHeader(
      queue, 0, FrameType::SETTINGS, ACK, 0, kNoPadding, folly::none, nullptr);
  return kFrameHeaderSize;
}

size_t writePushPromise(uint8_t* header,
                        size_t headerLen,
                        IOBufQueue& queue,
                        uint32_t associatedStream,
                        uint32_t promisedStream,
                        size_t headersLen,
                        folly::Optional<uint8_t> padding,
                        bool endHeaders) noexcept {
  DCHECK_NE(0, promisedStream);
  DCHECK_NE(0, associatedStream);
  DCHECK_EQ(0, 0x1 & promisedStream);
  DCHECK_EQ(1, 0x1 & associatedStream);
  DCHECK_EQ(0, ~kUint31Mask & promisedStream);

  const auto frameLen = writeFrameHeader(header,
                                         headerLen,
                                         headersLen + kFramePushPromiseSize,
                                         FrameType::PUSH_PROMISE,
                                         endHeaders ? END_HEADERS : 0,
                                         associatedStream,
                                         padding,
                                         folly::none);
  promisedStream = htonl(promisedStream);
  uint8_t* psPtr = header + kFrameHeaderSize;
  if (padding) {
    psPtr++;
  }
  memcpy(psPtr, &promisedStream, sizeof(promisedStream));
  writePadding(queue, padding);
  return kFrameHeaderSize + frameLen;
}

size_t writePing(IOBufQueue& queue, uint64_t opaqueData, bool ack) noexcept {
  const auto frameLen = writeFrameHeader(queue,
                                         kFramePingSize,
                                         FrameType::PING,
                                         ack ? ACK : 0,
                                         0,
                                         kNoPadding,
                                         folly::none,
                                         nullptr);
  queue.append(&opaqueData, sizeof(opaqueData));
  return kFrameHeaderSize + frameLen;
}

size_t writeGoaway(IOBufQueue& queue,
                   uint32_t lastStreamID,
                   ErrorCode errorCode,
                   std::unique_ptr<IOBuf> debugData) noexcept {
  uint32_t debugLen = debugData ? debugData->computeChainDataLength() : 0;
  DCHECK_EQ(0, ~kLengthMask & debugLen);
  const auto frameLen = writeFrameHeader(queue,
                                         kFrameGoawaySize + debugLen,
                                         FrameType::GOAWAY,
                                         0,
                                         0,
                                         kNoPadding,
                                         folly::none,
                                         nullptr);
  QueueAppender appender(&queue, frameLen);
  appender.writeBE<uint32_t>(lastStreamID);
  appender.writeBE<uint32_t>(static_cast<uint32_t>(errorCode));
  queue.append(std::move(debugData));
  return kFrameHeaderSize + frameLen;
}

size_t writeWindowUpdate(IOBufQueue& queue,
                         uint32_t stream,
                         uint32_t amount) noexcept {
  const auto frameLen = writeFrameHeader(queue,
                                         kFrameWindowUpdateSize,
                                         FrameType::WINDOW_UPDATE,
                                         0,
                                         stream,
                                         kNoPadding,
                                         folly::none,
                                         nullptr);
  DCHECK_EQ(0, ~kUint31Mask & amount);
  DCHECK_LT(0, amount);
  QueueAppender appender(&queue, kFrameWindowUpdateSize);
  appender.writeBE<uint32_t>(amount);
  return kFrameHeaderSize + frameLen;
}

size_t writeContinuation(IOBufQueue& queue,
                         uint32_t stream,
                         bool endHeaders,
                         std::unique_ptr<IOBuf> headers) noexcept {
  DCHECK_NE(0, stream);
  const auto dataLen = headers->computeChainDataLength();
  const auto frameLen = writeFrameHeader(queue,
                                         dataLen,
                                         FrameType::CONTINUATION,
                                         endHeaders ? END_HEADERS : 0,
                                         stream,
                                         kNoPadding,
                                         folly::none,
                                         std::move(headers));
  return kFrameHeaderSize + frameLen;
}

size_t writeAltSvc(IOBufQueue& queue,
                   uint32_t stream,
                   uint32_t maxAge,
                   uint16_t port,
                   StringPiece protocol,
                   StringPiece host,
                   StringPiece origin) noexcept {
  const auto protoLen = protocol.size();
  const auto hostLen = host.size();
  const auto originLen = origin.size();
  const auto frameLen = protoLen + hostLen + originLen + kFrameAltSvcSizeBase;

  writeFrameHeader(queue,
                   frameLen,
                   FrameType::ALTSVC,
                   0,
                   stream,
                   kNoPadding,
                   folly::none,
                   nullptr);
  QueueAppender appender(&queue, frameLen);
  appender.writeBE<uint32_t>(maxAge);
  appender.writeBE<uint16_t>(port);
  appender.writeBE<uint8_t>(protoLen);
  appender.push(reinterpret_cast<const uint8_t*>(protocol.data()), protoLen);
  appender.writeBE<uint8_t>(hostLen);
  appender.push(reinterpret_cast<const uint8_t*>(host.data()), hostLen);
  appender.push(reinterpret_cast<const uint8_t*>(origin.data()), originLen);
  return kFrameHeaderSize + frameLen;
}

size_t writeCertificateRequest(folly::IOBufQueue& writeBuf,
                               uint16_t requestId,
                               std::unique_ptr<folly::IOBuf> authRequest) {
  const auto dataLen = authRequest ? kFrameCertificateRequestSizeBase +
                                         authRequest->computeChainDataLength()
                                   : kFrameCertificateRequestSizeBase;
  // The CERTIFICATE_REQUEST frame must be sent on stream 0.
  const auto frameLen = writeFrameHeader(writeBuf,
                                         dataLen,
                                         FrameType::CERTIFICATE_REQUEST,
                                         0,
                                         0,
                                         kNoPadding,
                                         folly::none,
                                         nullptr);
  QueueAppender appender(&writeBuf, frameLen);
  appender.writeBE<uint16_t>(requestId);
  writeBuf.append(std::move(authRequest));
  return kFrameHeaderSize + frameLen;
}

size_t writeCertificate(folly::IOBufQueue& writeBuf,
                        uint16_t certId,
                        std::unique_ptr<folly::IOBuf> authenticator,
                        bool toBeContinued) {
  uint8_t flags = 0;
  if (toBeContinued) {
    flags |= TO_BE_CONTINUED;
  }
  const auto dataLen =
      authenticator
          ? kFrameCertificateSizeBase + authenticator->computeChainDataLength()
          : kFrameCertificateSizeBase;
  // The CERTIFICATE_REQUEST frame must be sent on stream 0.
  const auto frameLen = writeFrameHeader(writeBuf,
                                         dataLen,
                                         FrameType::CERTIFICATE,
                                         flags,
                                         0,
                                         kNoPadding,
                                         folly::none,
                                         nullptr);
  QueueAppender appender(&writeBuf, frameLen);
  appender.writeBE<uint16_t>(certId);
  writeBuf.append(std::move(authenticator));
  return kFrameHeaderSize + frameLen;
}

const char* getFrameTypeString(FrameType type) {
  switch (type) {
    case FrameType::DATA:
      return "DATA";
    case FrameType::HEADERS:
      return "HEADERS";
    case FrameType::PRIORITY:
      return "PRIORITY";
    case FrameType::RST_STREAM:
      return "RST_STREAM";
    case FrameType::SETTINGS:
      return "SETTINGS";
    case FrameType::PUSH_PROMISE:
      return "PUSH_PROMISE";
    case FrameType::PING:
      return "PING";
    case FrameType::GOAWAY:
      return "GOAWAY";
    case FrameType::WINDOW_UPDATE:
      return "WINDOW_UPDATE";
    case FrameType::CONTINUATION:
      return "CONTINUATION";
    case FrameType::ALTSVC:
      return "ALTSVC";
    case FrameType::CERTIFICATE_REQUEST:
      return "CERTIFICATE_REQUEST";
    case FrameType::CERTIFICATE:
      return "CERTIFICATE";
    default:
      // can happen when type was cast from uint8_t
      return "Unknown";
  }
  LOG(FATAL) << "Unreachable";
  return "";
}
}} // namespace proxygen::http2
