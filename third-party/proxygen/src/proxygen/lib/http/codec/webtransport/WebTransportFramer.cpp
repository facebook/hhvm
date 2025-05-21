/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportFramer.h>

namespace {
const size_t kDefaultBufferGrowth = 32;
}

namespace proxygen {

folly::Expected<PaddingCapsule, CapsuleCodec::ErrorCode> parsePadding(
    folly::io::Cursor& cursor, size_t length) {
  cursor.skip(length);
  return PaddingCapsule{length};
}

folly::Expected<WTResetStreamCapsule, CapsuleCodec::ErrorCode>
parseWTResetStream(folly::io::Cursor& cursor, size_t length) {
  WTResetStreamCapsule wtResetStreamCapsule{};
  auto streamIdOpt = quic::decodeQuicInteger(cursor, length);
  if (!streamIdOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= streamIdOpt->second;
  wtResetStreamCapsule.streamId = streamIdOpt->first;
  auto appProtocolErrorCodeOpt = quic::decodeQuicInteger(cursor, length);
  if (!appProtocolErrorCodeOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= appProtocolErrorCodeOpt->second;
  wtResetStreamCapsule.appProtocolErrorCode =
      static_cast<uint32_t>(appProtocolErrorCodeOpt->first);
  auto reliableSizeOpt = quic::decodeQuicInteger(cursor, length);
  if (!reliableSizeOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= reliableSizeOpt->second;
  wtResetStreamCapsule.reliableSize = reliableSizeOpt->first;
  if (length > 0) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
  return wtResetStreamCapsule;
}

folly::Expected<WTStopSendingCapsule, CapsuleCodec::ErrorCode>
parseWTStopSending(folly::io::Cursor& cursor, size_t length) {
  WTStopSendingCapsule wtStopSendingCapsule{};
  auto streamIdOpt = quic::decodeQuicInteger(cursor, length);
  if (!streamIdOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= streamIdOpt->second;
  wtStopSendingCapsule.streamId = streamIdOpt->first;
  auto appProtocolErrorCodeOpt = quic::decodeQuicInteger(cursor, length);
  if (!appProtocolErrorCodeOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= appProtocolErrorCodeOpt->second;
  wtStopSendingCapsule.appProtocolErrorCode =
      static_cast<uint32_t>(appProtocolErrorCodeOpt->first);
  if (length > 0) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
  return wtStopSendingCapsule;
}

folly::Expected<WTStreamCapsule, CapsuleCodec::ErrorCode> parseWTStream(
    folly::io::Cursor& cursor, size_t length, bool fin) {
  WTStreamCapsule wtStreamCapsule{};
  auto streamIdOpt = quic::decodeQuicInteger(cursor, length);
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
  auto maximumDataOpt = quic::decodeQuicInteger(cursor, length);
  if (!maximumDataOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumDataOpt->second;
  wtMaxDataCapsule.maximumData = maximumDataOpt->first;
  if (length > 0) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
  return wtMaxDataCapsule;
}

folly::Expected<WTMaxStreamDataCapsule, CapsuleCodec::ErrorCode>
parseWTMaxStreamData(folly::io::Cursor& cursor, size_t length) {
  WTMaxStreamDataCapsule wtMaxStreamDataCapsule{};
  auto streamIdOpt = quic::decodeQuicInteger(cursor, length);
  if (!streamIdOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= streamIdOpt->second;
  wtMaxStreamDataCapsule.streamId = streamIdOpt->first;
  auto maximumStreamDataOpt = quic::decodeQuicInteger(cursor, length);
  if (!maximumStreamDataOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumStreamDataOpt->second;
  wtMaxStreamDataCapsule.maximumStreamData = maximumStreamDataOpt->first;
  if (length > 0) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
  return wtMaxStreamDataCapsule;
}

folly::Expected<WTMaxStreamsCapsule, CapsuleCodec::ErrorCode> parseWTMaxStreams(
    folly::io::Cursor& cursor, size_t length) {
  WTMaxStreamsCapsule wtMaxStreamsCapsule{};
  auto maximumStreamsOpt = quic::decodeQuicInteger(cursor, length);
  if (!maximumStreamsOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumStreamsOpt->second;
  wtMaxStreamsCapsule.maximumStreams = maximumStreamsOpt->first;
  if (length > 0) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
  return wtMaxStreamsCapsule;
}

folly::Expected<WTDataBlockedCapsule, CapsuleCodec::ErrorCode>
parseWTDataBlocked(folly::io::Cursor& cursor, size_t length) {
  WTDataBlockedCapsule wtDataBlockedCapsule{};
  auto maximumDataOpt = quic::decodeQuicInteger(cursor, length);
  if (!maximumDataOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumDataOpt->second;
  wtDataBlockedCapsule.maximumData = maximumDataOpt->first;
  if (length > 0) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
  return wtDataBlockedCapsule;
}

folly::Expected<WTStreamDataBlockedCapsule, CapsuleCodec::ErrorCode>
parseWTStreamDataBlocked(folly::io::Cursor& cursor, size_t length) {
  WTStreamDataBlockedCapsule wtStreamDataBlockedCapsule{};
  auto streamIdOpt = quic::decodeQuicInteger(cursor, length);
  if (!streamIdOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= streamIdOpt->second;
  wtStreamDataBlockedCapsule.streamId = streamIdOpt->first;
  auto maximumStreamDataOpt = quic::decodeQuicInteger(cursor, length);
  if (!maximumStreamDataOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumStreamDataOpt->second;
  wtStreamDataBlockedCapsule.maximumStreamData = maximumStreamDataOpt->first;
  if (length > 0) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
  return wtStreamDataBlockedCapsule;
}

folly::Expected<WTStreamsBlockedCapsule, CapsuleCodec::ErrorCode>
parseWTStreamsBlocked(folly::io::Cursor& cursor, size_t length) {
  WTStreamsBlockedCapsule wtStreamsBlockedCapsule{};
  auto maximumStreamsOpt = quic::decodeQuicInteger(cursor, length);
  if (!maximumStreamsOpt) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
  }
  length -= maximumStreamsOpt->second;
  wtStreamsBlockedCapsule.maximumStreams = maximumStreamsOpt->first;
  if (length > 0) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
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
  auto errorCodeOpt = quic::decodeQuicInteger(cursor, length);
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
  if (length != 0) {
    return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
  return drainWebTransportSessionCapsule;
}

namespace {
// Helper to write a variable-length integer
void writeVarint(folly::IOBufQueue& buf,
                 uint64_t value,
                 size_t& size,
                 bool& error) noexcept {
  if (error) {
    return;
  }
  folly::io::QueueAppender appender(&buf, kDefaultBufferGrowth);
  auto appenderOp = [&](auto val) mutable { appender.writeBE(val); };
  auto res = quic::encodeQuicInteger(value, appenderOp);
  if (res.hasError()) {
    error = true;
  } else {
    size += *res;
  }
}

// Helper to write the capsule type and length
void writeCapsuleHeader(folly::IOBufQueue& queue,
                        CapsuleType capsuleType,
                        size_t& size,
                        bool& error,
                        uint64_t length) {
  writeVarint(queue, folly::to_underlying(capsuleType), size, error);
  writeVarint(queue, length, size, error);
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
      return res;
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
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeWTResetStream(folly::IOBufQueue& queue,
                               const WTResetStreamCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<3, 0>(
      {capsule.streamId, capsule.appProtocolErrorCode, capsule.reliableSize},
      {});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  writeCapsuleHeader(
      queue, CapsuleType::WT_RESET_STREAM, size, error, capsuleLen.value());
  writeVarint(queue, capsule.streamId, size, error);
  writeVarint(queue, capsule.appProtocolErrorCode, size, error);
  writeVarint(queue, capsule.reliableSize, size, error);
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeWTStopSending(folly::IOBufQueue& queue,
                               const WTStopSendingCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<2, 0>(
      {capsule.streamId, capsule.appProtocolErrorCode}, {});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  writeCapsuleHeader(
      queue, CapsuleType::WT_STOP_SENDING, size, error, capsuleLen.value());
  writeVarint(queue, capsule.streamId, size, error);
  writeVarint(queue, capsule.appProtocolErrorCode, size, error);
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeWTStream(folly::IOBufQueue& queue,
                          const WTStreamCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleType =
      capsule.fin ? CapsuleType::WT_STREAM_WITH_FIN : CapsuleType::WT_STREAM;
  auto capsuleLen = getCapsuleSize<1, 1>(
      {capsule.streamId}, {capsule.streamData->computeChainDataLength()});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  writeCapsuleHeader(queue, capsuleType, size, error, capsuleLen.value());
  writeVarint(queue, capsule.streamId, size, error);
  queue.append(capsule.streamData->clone());
  size += capsule.streamData->computeChainDataLength();
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeWTMaxData(folly::IOBufQueue& queue,
                           const WTMaxDataCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<1, 0>({capsule.maximumData}, {});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  writeCapsuleHeader(
      queue, CapsuleType::WT_MAX_DATA, size, error, capsuleLen.value());
  writeVarint(queue, capsule.maximumData, size, error);
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeWTMaxStreamData(folly::IOBufQueue& queue,
                                 const WTMaxStreamDataCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen =
      getCapsuleSize<2, 0>({capsule.streamId, capsule.maximumStreamData}, {});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  writeCapsuleHeader(
      queue, CapsuleType::WT_MAX_STREAM_DATA, size, error, capsuleLen.value());
  writeVarint(queue, capsule.streamId, size, error);
  writeVarint(queue, capsule.maximumStreamData, size, error);
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeWTMaxStreams(folly::IOBufQueue& queue,
                              const WTMaxStreamsCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<1, 0>({capsule.maximumStreams}, {});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  writeCapsuleHeader(
      queue, CapsuleType::WT_MAX_STREAMS, size, error, capsuleLen.value());
  writeVarint(queue, capsule.maximumStreams, size, error);
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeWTDataBlocked(folly::IOBufQueue& queue,
                               const WTDataBlockedCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<1, 0>({capsule.maximumData}, {});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  writeCapsuleHeader(
      queue, CapsuleType::WT_DATA_BLOCKED, size, error, capsuleLen.value());
  writeVarint(queue, capsule.maximumData, size, error);
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeWTStreamDataBlocked(
    folly::IOBufQueue& queue, const WTStreamDataBlockedCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen =
      getCapsuleSize<2, 0>({capsule.streamId, capsule.maximumStreamData}, {});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  writeCapsuleHeader(queue,
                     CapsuleType::WT_STREAM_DATA_BLOCKED,
                     size,
                     error,
                     capsuleLen.value());
  writeVarint(queue, capsule.streamId, size, error);
  writeVarint(queue, capsule.maximumStreamData, size, error);
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeWTStreamsBlocked(folly::IOBufQueue& queue,
                                  const WTStreamsBlockedCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<1, 0>({capsule.maximumStreams}, {});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  writeCapsuleHeader(
      queue, CapsuleType::WT_STREAMS_BLOCKED, size, error, capsuleLen.value());
  writeVarint(queue, capsule.maximumStreams, size, error);
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeDatagram(folly::IOBufQueue& queue,
                          const DatagramCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<0, 1>(
      {}, {capsule.httpDatagramPayload->computeChainDataLength()});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  writeCapsuleHeader(
      queue, CapsuleType::DATAGRAM, size, error, capsuleLen.value());
  queue.append(capsule.httpDatagramPayload->clone());
  size += capsule.httpDatagramPayload->computeChainDataLength();
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeCloseWebTransportSession(
    folly::IOBufQueue& queue, const CloseWebTransportSessionCapsule& capsule) {
  size_t size = 0;
  bool error = false;
  auto capsuleLen = getCapsuleSize<1, 1>(
      {capsule.applicationErrorCode}, {capsule.applicationErrorMessage.size()});
  if (capsuleLen.hasError()) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
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
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

WriteResult writeDrainWebTransportSession(folly::IOBufQueue& queue) {
  size_t size = 0;
  bool error = false;
  writeCapsuleHeader(
      queue, CapsuleType::DRAIN_WEBTRANSPORT_SESSION, size, error, 0);
  if (error) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  return size;
}

} // namespace proxygen
