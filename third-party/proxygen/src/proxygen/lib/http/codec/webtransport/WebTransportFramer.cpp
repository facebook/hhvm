/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/webtransport/WebTransportFramer.h>

#include <array>

#include <folly/Utility.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <proxygen/lib/http/codec/VarintUtils.h>

#include <quic/codec/QuicInteger.h>
#include <quic/folly_utils/Utils.h>

using proxygen::writeVarint;

namespace {
// Logical frame type index into kFrameTypeMap.
enum class FrameType : uint8_t {
  RESET_STREAM,
  STOP_SENDING,
  STREAM,
  STREAM_WITH_FIN,
  MAX_DATA,
  MAX_STREAM_DATA,
  MAX_STREAMS_BIDI,
  MAX_STREAMS_UNI,
  DATA_BLOCKED,
  STREAM_DATA_BLOCKED,
  STREAMS_BLOCKED_BIDI,
  STREAMS_BLOCKED_UNI,
  DATAGRAM,
  NUM_TYPES,
};

// Each entry is {WT capsule type, QMUX frame type}.
#define WT_QMUX(name)                                      \
  {folly::to_underlying(proxygen::CapsuleType::WT_##name), \
   folly::to_underlying(proxygen::QmuxFrameType::name)}

constexpr std::array<std::pair<uint64_t, uint64_t>,
                     static_cast<size_t>(FrameType::NUM_TYPES)>
    kFrameTypeMap = {{
        WT_QMUX(RESET_STREAM),
        WT_QMUX(STOP_SENDING),
        WT_QMUX(STREAM),
        WT_QMUX(STREAM_WITH_FIN),
        WT_QMUX(MAX_DATA),
        WT_QMUX(MAX_STREAM_DATA),
        WT_QMUX(MAX_STREAMS_BIDI),
        WT_QMUX(MAX_STREAMS_UNI),
        WT_QMUX(DATA_BLOCKED),
        WT_QMUX(STREAM_DATA_BLOCKED),
        WT_QMUX(STREAMS_BLOCKED_BIDI),
        WT_QMUX(STREAMS_BLOCKED_UNI),
        {folly::to_underlying(proxygen::CapsuleType::DATAGRAM),
         folly::to_underlying(proxygen::QmuxFrameType::DATAGRAM)},
    }};
#undef WT_QMUX

uint64_t getWireType(FrameType type, proxygen::FrameProtocol protocol) {
  const auto& [wt, qmux] = kFrameTypeMap[static_cast<size_t>(type)];
  return protocol == proxygen::FrameProtocol::QMUX ? qmux : wt;
}
} // namespace

namespace proxygen {

folly::Expected<PaddingCapsule, CapsuleCodec::ErrorCode> parsePadding(
    folly::io::Cursor& cursor, size_t length) {
  cursor.skip(length);
  return PaddingCapsule{length};
}

folly::Expected<WTResetStreamCapsule, CapsuleCodec::ErrorCode>
parseWTResetStream(folly::io::Cursor& cursor, size_t length) {
  WTResetStreamCapsule wtResetStreamCapsule{};
  auto streamIdOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!streamIdOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= streamIdOpt->second;
  wtResetStreamCapsule.streamId = streamIdOpt->first;
  auto appProtocolErrorCodeOpt =
      quic::follyutils::decodeQuicInteger(cursor, length);
  if (!appProtocolErrorCodeOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= appProtocolErrorCodeOpt->second;
  wtResetStreamCapsule.appProtocolErrorCode =
      static_cast<uint32_t>(appProtocolErrorCodeOpt->first);
  auto reliableSizeOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!reliableSizeOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= reliableSizeOpt->second;
  wtResetStreamCapsule.reliableSize = reliableSizeOpt->first;
  return wtResetStreamCapsule;
}

folly::Expected<WTStopSendingCapsule, CapsuleCodec::ErrorCode>
parseWTStopSending(folly::io::Cursor& cursor, size_t length) {
  WTStopSendingCapsule wtStopSendingCapsule{};
  auto streamIdOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!streamIdOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= streamIdOpt->second;
  wtStopSendingCapsule.streamId = streamIdOpt->first;
  auto appProtocolErrorCodeOpt =
      quic::follyutils::decodeQuicInteger(cursor, length);
  if (!appProtocolErrorCodeOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= appProtocolErrorCodeOpt->second;
  wtStopSendingCapsule.appProtocolErrorCode =
      static_cast<uint32_t>(appProtocolErrorCodeOpt->first);
  return wtStopSendingCapsule;
}

folly::Expected<WTStreamCapsule, CapsuleCodec::ErrorCode> parseWTStream(
    folly::io::Cursor& cursor, size_t length, bool fin) {
  WTStreamCapsule wtStreamCapsule{};
  auto streamIdOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!streamIdOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= streamIdOpt->second;
  wtStreamCapsule.streamId = streamIdOpt->first;
  if (!cursor.canAdvance(length)) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  cursor.cloneAtMost(wtStreamCapsule.streamData, length);
  wtStreamCapsule.fin = fin;
  return wtStreamCapsule;
}

folly::Expected<WTMaxDataCapsule, CapsuleCodec::ErrorCode> parseWTMaxData(
    folly::io::Cursor& cursor, size_t length) {
  WTMaxDataCapsule wtMaxDataCapsule{};
  auto maximumDataOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!maximumDataOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumDataOpt->second;
  wtMaxDataCapsule.maximumData = maximumDataOpt->first;
  return wtMaxDataCapsule;
}

folly::Expected<WTMaxStreamDataCapsule, CapsuleCodec::ErrorCode>
parseWTMaxStreamData(folly::io::Cursor& cursor, size_t length) {
  WTMaxStreamDataCapsule wtMaxStreamDataCapsule{};
  auto streamIdOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!streamIdOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= streamIdOpt->second;
  wtMaxStreamDataCapsule.streamId = streamIdOpt->first;
  auto maximumStreamDataOpt =
      quic::follyutils::decodeQuicInteger(cursor, length);
  if (!maximumStreamDataOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumStreamDataOpt->second;
  wtMaxStreamDataCapsule.maximumStreamData = maximumStreamDataOpt->first;
  return wtMaxStreamDataCapsule;
}

folly::Expected<WTMaxStreamsCapsule, CapsuleCodec::ErrorCode> parseWTMaxStreams(
    folly::io::Cursor& cursor, size_t length) {
  WTMaxStreamsCapsule wtMaxStreamsCapsule{};
  auto maximumStreamsOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!maximumStreamsOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumStreamsOpt->second;
  wtMaxStreamsCapsule.maximumStreams = maximumStreamsOpt->first;
  return wtMaxStreamsCapsule;
}

folly::Expected<WTDataBlockedCapsule, CapsuleCodec::ErrorCode>
parseWTDataBlocked(folly::io::Cursor& cursor, size_t length) {
  WTDataBlockedCapsule wtDataBlockedCapsule{};
  auto maximumDataOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!maximumDataOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumDataOpt->second;
  wtDataBlockedCapsule.maximumData = maximumDataOpt->first;
  return wtDataBlockedCapsule;
}

folly::Expected<WTStreamDataBlockedCapsule, CapsuleCodec::ErrorCode>
parseWTStreamDataBlocked(folly::io::Cursor& cursor, size_t length) {
  WTStreamDataBlockedCapsule wtStreamDataBlockedCapsule{};
  auto streamIdOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!streamIdOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= streamIdOpt->second;
  wtStreamDataBlockedCapsule.streamId = streamIdOpt->first;
  auto maximumStreamDataOpt =
      quic::follyutils::decodeQuicInteger(cursor, length);
  if (!maximumStreamDataOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumStreamDataOpt->second;
  wtStreamDataBlockedCapsule.maximumStreamData = maximumStreamDataOpt->first;
  return wtStreamDataBlockedCapsule;
}

folly::Expected<WTStreamsBlockedCapsule, CapsuleCodec::ErrorCode>
parseWTStreamsBlocked(folly::io::Cursor& cursor, size_t length) {
  WTStreamsBlockedCapsule wtStreamsBlockedCapsule{};
  auto maximumStreamsOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!maximumStreamsOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumStreamsOpt->second;
  wtStreamsBlockedCapsule.maximumStreams = maximumStreamsOpt->first;
  return wtStreamsBlockedCapsule;
}

folly::Expected<DatagramCapsule, CapsuleCodec::ErrorCode> parseDatagram(
    folly::io::Cursor& cursor, size_t length) {
  DatagramCapsule datagramCapsule{};
  if (!cursor.canAdvance(length)) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  cursor.cloneAtMost(datagramCapsule.httpDatagramPayload, length);
  return datagramCapsule;
}

folly::Expected<CloseWebTransportSessionCapsule, CapsuleCodec::ErrorCode>
parseCloseWebTransportSession(folly::io::Cursor& cursor, size_t length) {
  CloseWebTransportSessionCapsule closeWebTransportSessionCapsule{};
  auto errorCodeOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!errorCodeOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  if (errorCodeOpt->first > std::numeric_limits<uint32_t>::max()) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
  length -= errorCodeOpt->second;
  closeWebTransportSessionCapsule.applicationErrorCode =
      static_cast<uint32_t>(errorCodeOpt->first);
  if (!cursor.canAdvance(length)) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  closeWebTransportSessionCapsule.applicationErrorMessage.resize(length);
  cursor.pull(closeWebTransportSessionCapsule.applicationErrorMessage.data(),
              length);
  return closeWebTransportSessionCapsule;
}

folly::Expected<DrainWebTransportSessionCapsule, CapsuleCodec::ErrorCode>
parseDrainWebTransportSession(size_t length) {
  DrainWebTransportSessionCapsule drainWebTransportSessionCapsule{};
  return drainWebTransportSessionCapsule;
}

namespace {
// Helper to write the capsule type and length
void writeCapsuleHeader(folly::IOBufQueue& queue,
                        CapsuleType capsuleType,
                        size_t& size,
                        bool& error,
                        uint64_t length) {
  writeVarint(queue, folly::to_underlying(capsuleType), size, error);
  writeVarint(queue, length, size, error);
}

// Writes the appropriate frame header based on protocol.
// WT_CAPSULE: writes capsule type + payload length (TLV).
// QMUX: writes just the frame type (no length prefix).
void writeFrameHeader(folly::IOBufQueue& queue,
                      proxygen::FrameProtocol protocol,
                      FrameType type,
                      size_t& size,
                      bool& error,
                      uint64_t payloadLength) {
  auto wireType = getWireType(type, protocol);
  if (protocol == proxygen::FrameProtocol::QMUX) {
    writeVarint(queue, wireType, size, error);
  } else {
    writeVarint(queue, wireType, size, error);
    writeVarint(queue, payloadLength, size, error);
  }
}

// Helper to calculate the capsule size based on varint encoding + actual
// payload size for string fields
template <const size_t N, const size_t M>
folly::Expected<uint64_t, quic::QuicError> getCapsuleSize(
    std::array<uint64_t, N> varints, std::array<uint64_t, M> lengths = {}) {
  size_t size = 0;
  for (auto v : varints) {
    auto res = quic::getQuicIntegerSize(v);
    if (!res) {
      return folly::makeUnexpected(res.error());
    }
    size += *res;
  }
  for (auto l : lengths) {
    size += l;
  }
  return size;
}
} // namespace

WriteResult writePadding(folly::IOBufQueue& queue,
                         const PaddingCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  writeCapsuleHeader(
      queue, CapsuleType::PADDING, size, error, capsule.paddingLength);
  std::string padding(capsule.paddingLength, 0);
  queue.append(padding.data(), padding.size());
  size += padding.size();
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeWTResetStream(folly::IOBufQueue& queue,
                               const WTResetStreamCapsule& capsule,
                               FrameProtocol protocol) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<3, 0>(
      {capsule.streamId, capsule.appProtocolErrorCode, capsule.reliableSize},
      {});
  if (capsuleLen.hasError()) {
    return std::nullopt;
  }
  writeFrameHeader(queue,
                   protocol,
                   FrameType::RESET_STREAM,
                   size,
                   error,
                   capsuleLen.value());
  writeVarint(queue, capsule.streamId, size, error);
  writeVarint(queue, capsule.appProtocolErrorCode, size, error);
  writeVarint(queue, capsule.reliableSize, size, error);
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeWTStopSending(folly::IOBufQueue& queue,
                               const WTStopSendingCapsule& capsule,
                               FrameProtocol protocol) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<2, 0>(
      {capsule.streamId, capsule.appProtocolErrorCode}, {});
  if (capsuleLen.hasError()) {
    return std::nullopt;
  }
  writeFrameHeader(queue,
                   protocol,
                   FrameType::STOP_SENDING,
                   size,
                   error,
                   capsuleLen.value());
  writeVarint(queue, capsule.streamId, size, error);
  writeVarint(queue, capsule.appProtocolErrorCode, size, error);
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeWTStream(folly::IOBufQueue& queue,
                          const WTStreamCapsule& capsule,
                          FrameProtocol protocol) {
  size_t size = 0;
  bool error = false;
  auto frameType = capsule.fin ? FrameType::STREAM_WITH_FIN : FrameType::STREAM;
  const auto dataLen =
      capsule.streamData ? capsule.streamData->computeChainDataLength() : 0;
  if (protocol == FrameProtocol::QMUX) {
    // QMUX STREAM: type (includes LEN flag) + streamId + dataLen + data
    writeVarint(queue, getWireType(frameType, protocol), size, error);
    writeVarint(queue, capsule.streamId, size, error);
    writeVarint(queue, dataLen, size, error);
  } else {
    // WT capsule: type + capsuleLen + streamId + data
    auto capsuleLen = getCapsuleSize<1, 1>({capsule.streamId}, {dataLen});
    if (capsuleLen.hasError()) {
      return std::nullopt;
    }
    writeFrameHeader(
        queue, protocol, frameType, size, error, capsuleLen.value());
    writeVarint(queue, capsule.streamId, size, error);
  }
  if (dataLen > 0) {
    queue.append(capsule.streamData->clone());
    size += dataLen;
  }
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeWTMaxData(folly::IOBufQueue& queue,
                           const WTMaxDataCapsule& capsule,
                           FrameProtocol protocol) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<1, 0>({capsule.maximumData}, {});
  if (capsuleLen.hasError()) {
    return std::nullopt;
  }
  writeFrameHeader(
      queue, protocol, FrameType::MAX_DATA, size, error, capsuleLen.value());
  writeVarint(queue, capsule.maximumData, size, error);
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeWTMaxStreamData(folly::IOBufQueue& queue,
                                 const WTMaxStreamDataCapsule& capsule,
                                 FrameProtocol protocol) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen =
      getCapsuleSize<2, 0>({capsule.streamId, capsule.maximumStreamData}, {});
  if (capsuleLen.hasError()) {
    return std::nullopt;
  }
  writeFrameHeader(queue,
                   protocol,
                   FrameType::MAX_STREAM_DATA,
                   size,
                   error,
                   capsuleLen.value());
  writeVarint(queue, capsule.streamId, size, error);
  writeVarint(queue, capsule.maximumStreamData, size, error);
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeWTMaxStreams(folly::IOBufQueue& queue,
                              const WTMaxStreamsCapsule& capsule,
                              bool isBidi,
                              FrameProtocol protocol) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<1, 0>({capsule.maximumStreams}, {});
  if (capsuleLen.hasError()) {
    return std::nullopt;
  }
  auto type = isBidi ? FrameType::MAX_STREAMS_BIDI : FrameType::MAX_STREAMS_UNI;
  writeFrameHeader(queue, protocol, type, size, error, capsuleLen.value());
  writeVarint(queue, capsule.maximumStreams, size, error);
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeWTDataBlocked(folly::IOBufQueue& queue,
                               const WTDataBlockedCapsule& capsule,
                               FrameProtocol protocol) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<1, 0>({capsule.maximumData}, {});
  if (capsuleLen.hasError()) {
    return std::nullopt;
  }
  writeFrameHeader(queue,
                   protocol,
                   FrameType::DATA_BLOCKED,
                   size,
                   error,
                   capsuleLen.value());
  writeVarint(queue, capsule.maximumData, size, error);
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeWTStreamDataBlocked(folly::IOBufQueue& queue,
                                     const WTStreamDataBlockedCapsule& capsule,
                                     FrameProtocol protocol) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen =
      getCapsuleSize<2, 0>({capsule.streamId, capsule.maximumStreamData}, {});
  if (capsuleLen.hasError()) {
    return std::nullopt;
  }
  writeFrameHeader(queue,
                   protocol,
                   FrameType::STREAM_DATA_BLOCKED,
                   size,
                   error,
                   capsuleLen.value());
  writeVarint(queue, capsule.streamId, size, error);
  writeVarint(queue, capsule.maximumStreamData, size, error);
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeWTStreamsBlocked(folly::IOBufQueue& queue,
                                  const WTStreamsBlockedCapsule& capsule,
                                  bool isBidi,
                                  FrameProtocol protocol) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<1, 0>({capsule.maximumStreams}, {});
  if (capsuleLen.hasError()) {
    return std::nullopt;
  }
  auto type =
      isBidi ? FrameType::STREAMS_BLOCKED_BIDI : FrameType::STREAMS_BLOCKED_UNI;
  writeFrameHeader(queue, protocol, type, size, error, capsuleLen.value());
  writeVarint(queue, capsule.maximumStreams, size, error);
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeDatagram(folly::IOBufQueue& queue,
                          const DatagramCapsule& capsule,
                          FrameProtocol protocol) {
  size_t size = 0;
  bool error = false;
  auto dataLen = capsule.httpDatagramPayload
                     ? capsule.httpDatagramPayload->computeChainDataLength()
                     : 0;
  if (protocol == FrameProtocol::QMUX) {
    // QMUX DATAGRAM_WITH_LEN: type + varint(dataLen) + data
    writeVarint(queue, getWireType(FrameType::DATAGRAM, protocol), size, error);
    writeVarint(queue, dataLen, size, error);
  } else {
    // WT capsule: type + capsuleLen + data
    auto capsuleLen = getCapsuleSize<0, 1>({}, {dataLen});
    if (capsuleLen.hasError()) {
      return std::nullopt;
    }
    writeFrameHeader(
        queue, protocol, FrameType::DATAGRAM, size, error, capsuleLen.value());
  }
  if (dataLen > 0) {
    queue.append(capsule.httpDatagramPayload->clone());
    size += dataLen;
  }
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeCloseWebTransportSession(
    folly::IOBufQueue& queue, const CloseWebTransportSessionCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<1, 1>(
      {capsule.applicationErrorCode}, {capsule.applicationErrorMessage.size()});
  if (capsuleLen.hasError()) {
    return std::nullopt;
  }
  writeCapsuleHeader(queue,
                     CapsuleType::CLOSE_WEBTRANSPORT_SESSION,
                     size,
                     error,
                     capsuleLen.value());
  writeVarint(queue, capsule.applicationErrorCode, size, error);
  queue.append(
      folly::IOBuf::copyBuffer(capsule.applicationErrorMessage.data(),
                               capsule.applicationErrorMessage.size()));
  size += capsule.applicationErrorMessage.size();
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeDrainWebTransportSession(folly::IOBufQueue& queue) {
  size_t size = 0;
  bool error = false;
  writeCapsuleHeader(
      queue, CapsuleType::DRAIN_WEBTRANSPORT_SESSION, size, error, 0);
  return error ? std::nullopt : std::make_optional(size);
}

} // namespace proxygen
