/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Random.h>

#include <proxygen/lib/http/HTTPPriorityFunctions.h>
#include <proxygen/lib/http/codec/CodecUtil.h>
#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HQUtils.h>
#include <quic/codec/QuicInteger.h>

using namespace folly::io;
using namespace folly;

namespace proxygen { namespace hq {

bool isGreaseId(uint64_t id) {
  if (id < 0x21 || id > quic::kEightByteLimit) {
    return false;
  }
  return (((id - 0x21) % 0x1F) == 0);
}

folly::Optional<uint64_t> getGreaseId(uint64_t n) {
  if (n > kMaxGreaseIdIndex) {
    return folly::none;
  }
  return (0x1F * n) + 0x21;
}

ParseResult parseData(folly::io::Cursor& cursor,
                      const FrameHeader& header,
                      std::unique_ptr<folly::IOBuf>& outBuf) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  cursor.clone(outBuf, header.length);
  return folly::none;
}

ParseResult parseHeaders(folly::io::Cursor& cursor,
                         const FrameHeader& header,
                         std::unique_ptr<folly::IOBuf>& outBuf) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  // for HEADERS frame, zero-length is allowed
  cursor.clone(outBuf, header.length);
  return folly::none;
}

static ParseResult parseIdOnlyFrame(folly::io::Cursor& cursor,
                                    const FrameHeader& header,
                                    uint64_t& outId) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  auto frameLength = header.length;

  auto id = quic::decodeQuicInteger(cursor, frameLength);
  if (!id) {
    return HTTP3::ErrorCode::HTTP_FRAME_ERROR;
  }
  outId = id->first;
  frameLength -= id->second;
  if (frameLength != 0) {
    return HTTP3::ErrorCode::HTTP_FRAME_ERROR;
  }

  return folly::none;
}

ParseResult parseCancelPush(folly::io::Cursor& cursor,
                            const FrameHeader& header,
                            PushId& outPushId) noexcept {
  return parseIdOnlyFrame(cursor, header, outPushId);
}

folly::Expected<folly::Optional<SettingValue>, HTTP3::ErrorCode>
decodeSettingValue(folly::io::Cursor& cursor,
                   uint64_t& frameLength,
                   SettingId settingId) {

  // read the setting value
  auto settingValue = quic::decodeQuicInteger(cursor, frameLength);
  if (!settingValue) {
    return folly::makeUnexpected(HTTP3::ErrorCode::HTTP_FRAME_ERROR);
  }
  auto value = settingValue->first;
  frameLength -= settingValue->second;

  // return the the value from the wire for known settings, folly::none for
  // unknown ones
  switch (settingId) {
    case SettingId::HEADER_TABLE_SIZE:
    case SettingId::MAX_HEADER_LIST_SIZE:
    case SettingId::QPACK_BLOCKED_STREAMS:
    case SettingId::ENABLE_CONNECT_PROTOCOL:
    case SettingId::H3_DATAGRAM:
    case SettingId::H3_DATAGRAM_DRAFT_8:
    case SettingId::H3_DATAGRAM_RFC:
    case SettingId::ENABLE_WEBTRANSPORT:
    case SettingId::WEBTRANSPORT_MAX_SESSIONS:
      return value;
  }
  return folly::none;
}

ParseResult parseSettings(folly::io::Cursor& cursor,
                          const FrameHeader& header,
                          std::deque<SettingPair>& settings) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  folly::IOBuf buf;
  auto frameLength = header.length;

  while (frameLength > 0) {
    auto settingIdRes = quic::decodeQuicInteger(cursor, frameLength);
    if (!settingIdRes) {
      return HTTP3::ErrorCode::HTTP_FRAME_ERROR;
    }
    frameLength -= settingIdRes->second;

    auto settingId = SettingId(settingIdRes->first);
    auto settingValue = decodeSettingValue(cursor, frameLength, settingId);
    if (settingValue.hasError()) {
      return settingValue.error();
    }

    // TODO: Duped id should trigger H3_SETTINGS_ERROR
    if (settingValue->has_value()) {
      settings.emplace_back(settingId, settingValue->value());
    }
  }
  return folly::none;
}

ParseResult parsePushPromise(folly::io::Cursor& cursor,
                             const FrameHeader& header,
                             PushId& outPushId,
                             std::unique_ptr<folly::IOBuf>& outBuf) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  folly::IOBuf buf;
  auto frameLength = header.length;

  auto pushId = quic::decodeQuicInteger(cursor, frameLength);
  if (!pushId) {
    return HTTP3::ErrorCode::HTTP_FRAME_ERROR;
  }
  outPushId = pushId->first;
  frameLength -= pushId->second;

  cursor.clone(outBuf, frameLength);
  return folly::none;
}

ParseResult parseGoaway(folly::io::Cursor& cursor,
                        const FrameHeader& header,
                        quic::StreamId& outStreamId) noexcept {
  return parseIdOnlyFrame(cursor, header, outStreamId);
}

ParseResult parseMaxPushId(folly::io::Cursor& cursor,
                           const FrameHeader& header,
                           quic::StreamId& outPushId) noexcept {
  return parseIdOnlyFrame(cursor, header, outPushId);
}

ParseResult parsePriorityUpdate(folly::io::Cursor& cursor,
                                const FrameHeader& header,
                                HTTPCodec::StreamID& outId,
                                HTTPPriority& priorityUpdate) noexcept {
  DCHECK_LE(header.length, cursor.totalLength());
  auto length = header.length;
  auto id = quic::decodeQuicInteger(cursor, length);
  if (!id) {
    return HTTP3::ErrorCode::HTTP_ID_ERROR;
  }
  outId = id->first;
  auto bufferLength = length - id->second;
  auto buf = folly::IOBuf::create(bufferLength);
  cursor.pull((void*)(buf->data()), bufferLength);
  buf->append(bufferLength);
  auto httpPriority = httpPriorityFromString(
      folly::StringPiece(folly::ByteRange(buf->data(), buf->length())));
  if (!httpPriority) {
    return HTTP3::ErrorCode::HTTP_FRAME_ERROR;
  }
  priorityUpdate = *httpPriority;
  return folly::none;
}

/**
 * Generate just the common frame header. Returns the total frame header length
 */
WriteResult writeFrameHeader(IOBufQueue& queue,
                             FrameType type,
                             uint64_t length) noexcept {
  QueueAppender appender(&queue, kMaxFrameHeaderSize);
  auto appenderOp = [appender = std::move(appender)](auto val) mutable {
    appender.writeBE(val);
  };
  auto typeRes =
      quic::encodeQuicInteger(static_cast<uint64_t>(type), appenderOp);
  if (typeRes.hasError()) {
    return typeRes;
  }
  auto lengthRes = quic::encodeQuicInteger(length, appenderOp);
  if (lengthRes.hasError()) {
    return lengthRes;
  }
  return *typeRes + *lengthRes;
}

WriteResult writeSimpleFrame(IOBufQueue& queue,
                             FrameType type,
                             std::unique_ptr<folly::IOBuf> data) noexcept {
  DCHECK(data);
  auto payloadSize = data->computeChainDataLength();
  auto headerSize = writeFrameHeader(queue, type, payloadSize);
  if (headerSize.hasError()) {
    return headerSize;
  }
  queue.append(std::move(data));
  return *headerSize + payloadSize;
}

WriteResult writeData(IOBufQueue& queue,
                      std::unique_ptr<folly::IOBuf> data) noexcept {
  return writeSimpleFrame(queue, FrameType::DATA, std::move(data));
}

WriteResult writeHeaders(IOBufQueue& queue,
                         std::unique_ptr<folly::IOBuf> data) noexcept {
  return writeSimpleFrame(queue, FrameType::HEADERS, std::move(data));
}

WriteResult writeCancelPush(folly::IOBufQueue& writeBuf,
                            PushId pushId) noexcept {
  auto pushIdSize = quic::getQuicIntegerSize(pushId);
  if (pushIdSize.hasError()) {
    return pushIdSize;
  }
  IOBufQueue queue{IOBufQueue::cacheChainLength()};
  QueueAppender appender(&queue, *pushIdSize);
  quic::encodeQuicInteger(pushId,
                          [appender = std::move(appender)](auto val) mutable {
                            appender.writeBE(val);
                          });
  return writeSimpleFrame(writeBuf, FrameType::CANCEL_PUSH, queue.move());
}

WriteResult writeSettings(IOBufQueue& queue,
                          const std::deque<SettingPair>& settings) {
  size_t settingsSize = 0;
  // iterate through the settings to compute the frame payload length
  for (const auto& setting : settings) {
    auto idSize =
        quic::getQuicIntegerSize(static_cast<uint64_t>(setting.first));
    if (idSize.hasError()) {
      return idSize;
    }
    auto valueSize = quic::getQuicIntegerSize(setting.second);
    if (valueSize.hasError()) {
      return valueSize;
    }
    settingsSize += *idSize + *valueSize;
  }
  // write the frame header
  auto headerSize = writeFrameHeader(queue, FrameType::SETTINGS, settingsSize);
  if (headerSize.hasError()) {
    return headerSize;
  }
  // write the frame payload
  QueueAppender appender(&queue, settingsSize);
  auto appenderOp = [appender = std::move(appender)](auto val) mutable {
    appender.writeBE(val);
  };
  for (const auto& setting : settings) {
    quic::encodeQuicInteger(static_cast<uint64_t>(setting.first), appenderOp);
    quic::encodeQuicInteger(setting.second, appenderOp);
  }
  return *headerSize + settingsSize;
}

WriteResult writePushPromise(IOBufQueue& queue,
                             PushId pushId,
                             std::unique_ptr<folly::IOBuf> data) noexcept {
  DCHECK(data);
  auto pushIdSize = quic::getQuicIntegerSize(pushId);
  if (pushIdSize.hasError()) {
    return pushIdSize;
  }
  size_t payloadSize = *pushIdSize + data->computeChainDataLength();
  auto headerSize =
      writeFrameHeader(queue, FrameType::PUSH_PROMISE, payloadSize);
  if (headerSize.hasError()) {
    return headerSize;
  }
  QueueAppender appender(&queue, payloadSize);
  quic::encodeQuicInteger(pushId, [&](auto val) { appender.writeBE(val); });
  appender.insert(std::move(data));
  return *headerSize + payloadSize;
}

WriteResult writeGoaway(folly::IOBufQueue& writeBuf,
                        quic::StreamId lastStreamId) noexcept {
  auto lastStreamIdSize = quic::getQuicIntegerSize(lastStreamId);
  if (lastStreamIdSize.hasError()) {
    return lastStreamIdSize;
  }
  IOBufQueue queue{IOBufQueue::cacheChainLength()};
  QueueAppender appender(&queue, *lastStreamIdSize);
  quic::encodeQuicInteger(lastStreamId,
                          [appender = std::move(appender)](auto val) mutable {
                            appender.writeBE(val);
                          });
  return writeSimpleFrame(writeBuf, FrameType::GOAWAY, queue.move());
}

WriteResult writeMaxPushId(folly::IOBufQueue& writeBuf,
                           PushId maxPushId) noexcept {
  auto maxPushIdSize = quic::getQuicIntegerSize(maxPushId);
  if (maxPushIdSize.hasError()) {
    return maxPushIdSize;
  }
  IOBufQueue queue{IOBufQueue::cacheChainLength()};
  QueueAppender appender(&queue, *maxPushIdSize);
  quic::encodeQuicInteger(maxPushId,
                          [appender = std::move(appender)](auto val) mutable {
                            appender.writeBE(val);
                          });
  return writeSimpleFrame(writeBuf, FrameType::MAX_PUSH_ID, queue.move());
}

WriteResult writePriorityUpdate(folly::IOBufQueue& writeBuf,
                                quic::StreamId streamId,
                                folly::StringPiece priorityUpdate) noexcept {
  auto type = FrameType::FB_PRIORITY_UPDATE;
  auto streamIdSize = quic::getQuicIntegerSize(streamId);
  if (streamIdSize.hasError()) {
    return streamIdSize;
  }
  IOBufQueue queue(IOBufQueue::cacheChainLength());
  QueueAppender appender(&queue, *streamIdSize);
  quic::encodeQuicInteger(streamId,
                          [&appender](auto val) { appender.writeBE(val); });
  appender.pushAtMost((const uint8_t*)(priorityUpdate.data()),
                      priorityUpdate.size());
  return writeSimpleFrame(writeBuf, type, queue.move());
}

WriteResult writePushPriorityUpdate(
    folly::IOBufQueue& writeBuf,
    hq::PushId pushId,
    folly::StringPiece priorityUpdate) noexcept {
  auto type = FrameType::FB_PUSH_PRIORITY_UPDATE;
  auto streamIdSize = quic::getQuicIntegerSize(pushId);
  if (streamIdSize.hasError()) {
    return streamIdSize;
  }
  IOBufQueue queue(IOBufQueue::cacheChainLength());
  QueueAppender appender(&queue, *streamIdSize);
  quic::encodeQuicInteger(pushId,
                          [&appender](auto val) { appender.writeBE(val); });
  appender.pushAtMost((const uint8_t*)(priorityUpdate.data()),
                      priorityUpdate.size());
  return writeSimpleFrame(writeBuf, type, queue.move());
}

WriteResult writeStreamPreface(folly::IOBufQueue& writeBuf,
                               uint64_t streamPreface) noexcept {
  auto streamPrefaceSize = quic::getQuicIntegerSize(streamPreface);
  if (streamPrefaceSize.hasError()) {
    return streamPrefaceSize;
  }
  QueueAppender appender(&writeBuf, *streamPrefaceSize);
  quic::encodeQuicInteger(streamPreface,
                          [&appender](auto val) { appender.writeBE(val); });
  return *streamPrefaceSize;
}

const char* getFrameTypeString(FrameType type) {
  switch (type) {
    case FrameType::DATA:
      return "DATA";
    case FrameType::HEADERS:
      return "HEADERS";
    case FrameType::CANCEL_PUSH:
      return "CANCEL_PUSH";
    case FrameType::SETTINGS:
      return "SETTINGS";
    case FrameType::PUSH_PROMISE:
      return "PUSH_PROMISE";
    case FrameType::GOAWAY:
      return "GOAWAY";
    case FrameType::MAX_PUSH_ID:
      return "MAX_PUSH_ID";
    case FrameType::PRIORITY_UPDATE:
    case FrameType::FB_PRIORITY_UPDATE:
      return "PRIORITY_UPDATE";
    case FrameType::PUSH_PRIORITY_UPDATE:
    case FrameType::FB_PUSH_PRIORITY_UPDATE:
      return "PUSH_PRIORITY_UPDATE";
    default:
      if (isGreaseId(static_cast<uint64_t>(type))) {
        return "GREASE";
      }
      // can happen when type was cast from uint8_t
      return "Unknown";
  }
  LOG(FATAL) << "Unreachable";
  return "";
}

std::ostream& operator<<(std::ostream& os, FrameType type) {
  os << getFrameTypeString(type);
  return os;
}

WriteResult writeGreaseFrame(folly::IOBufQueue& writeBuf) noexcept {
  auto greaseId = getGreaseId(folly::Random::rand32(16));
  if (!greaseId) {
    return folly::makeUnexpected(quic::TransportErrorCode::INTERNAL_ERROR);
  }
  uint64_t uiFrameType = *greaseId;
  auto frameTypeSize = quic::getQuicIntegerSize(uiFrameType);
  if (frameTypeSize.hasError()) {
    return frameTypeSize;
  }
  return writeFrameHeader(writeBuf, static_cast<FrameType>(uiFrameType), 0);
}

WriteResult writeWTStreamPreface(folly::IOBufQueue& writeBuf,
                                 WebTransportStreamType streamType,
                                 uint64_t wtSessionId) {
  static const std::array<uint64_t, 2> streamTypes{
      folly::to_underlying(UnidirectionalStreamType::WEBTRANSPORT),
      folly::to_underlying(BidirectionalStreamType::WEBTRANSPORT)};
  auto idx = folly::to_underlying(streamType);
  CHECK_GE(idx, 0);
  CHECK_LT(idx, streamTypes.size());
  QueueAppender appender(&writeBuf, 64);
  size_t prefaceSize = 0;
  auto res = quic::encodeQuicInteger(
      streamTypes[idx], [&appender](auto val) { appender.writeBE(val); });
  if (!res) {
    return res;
  }
  prefaceSize += res.value();
  res = quic::encodeQuicInteger(
      wtSessionId, [&appender](auto val) { appender.writeBE(val); });
  if (!res) {
    return res;
  }
  prefaceSize += res.value();
  return prefaceSize;
}

}} // namespace proxygen::hq
