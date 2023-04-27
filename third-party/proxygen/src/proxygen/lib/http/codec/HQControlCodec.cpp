/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HQControlCodec.h>

#include <proxygen/lib/http/HTTP3ErrorCode.h>
#include <proxygen/lib/http/codec/CodecUtil.h>
#include <proxygen/lib/http/codec/HQUtils.h>

#include <folly/Random.h>

namespace {
using namespace proxygen::hq;

uint64_t drainingId(proxygen::TransportDirection dir) {
  if (dir == proxygen::TransportDirection::DOWNSTREAM) {
    return kMaxClientBidiStreamId;
  } else {
    return kMaxPushId + 1;
  }
}

} // namespace

namespace proxygen { namespace hq {

using namespace folly::io;

ParseResult HQControlCodec::checkFrameAllowed(FrameType type) {
  switch (type) {
    case hq::FrameType::DATA:
    case hq::FrameType::HEADERS:
    case hq::FrameType::PUSH_PROMISE:
    case hq::FrameType::WEBTRANSPORT_BIDI:
      return HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED;
    default:
      break;
  }

  if (getStreamType() == hq::UnidirectionalStreamType::CONTROL) {
    // SETTINGS MUST be the first frame on an HQ Control Stream
    if (!receivedSettings_ && type != hq::FrameType::SETTINGS) {
      return HTTP3::ErrorCode::HTTP_MISSING_SETTINGS;
    }
    // multiple SETTINGS frames are not allowed
    if (receivedSettings_ && type == hq::FrameType::SETTINGS) {
      return HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED;
    }
    // A client MUST treat the receipt of a MAX_PUSH_ID frame as a connection
    // error of type HTTP_FRAME_UNEXPECTED
    if (transportDirection_ == TransportDirection::UPSTREAM &&
        type == hq::FrameType::MAX_PUSH_ID) {
      return HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED;
    }

    // PRIORITY_UPDATE is downstream control codec only
    if (transportDirection_ == TransportDirection::UPSTREAM &&
        (type == hq::FrameType::PRIORITY_UPDATE ||
         type == hq::FrameType::PUSH_PRIORITY_UPDATE ||
         type == hq::FrameType::FB_PUSH_PRIORITY_UPDATE ||
         type == hq::FrameType::FB_PRIORITY_UPDATE)) {
      return HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED;
    }
  }

  return folly::none;
}

ParseResult HQControlCodec::parseCancelPush(Cursor& cursor,
                                            const FrameHeader& header) {
  PushId outPushId;
  auto res = hq::parseCancelPush(cursor, header, outPushId);
  return res;
}

ParseResult HQControlCodec::parseSettings(Cursor& cursor,
                                          const FrameHeader& header) {
  VLOG(4) << "parsing SETTINGS frame length=" << header.length;
  CHECK(isIngress());
  std::deque<SettingPair> outSettings;
  receivedSettings_ = true;
  auto res = hq::parseSettings(cursor, header, outSettings);
  VLOG(4) << "Received n=" << outSettings.size() << " settings";
  if (res) {
    return res;
  }

  CHECK(isIngress());
  auto& ingressSettings = settings_;
  SettingsList settingsList;
  for (auto& setting : outSettings) {
    switch (setting.first) {
      case hq::SettingId::HEADER_TABLE_SIZE:
      case hq::SettingId::MAX_HEADER_LIST_SIZE:
      case hq::SettingId::QPACK_BLOCKED_STREAMS:
      case hq::SettingId::WEBTRANSPORT_MAX_SESSIONS:
        break;
      case hq::SettingId::ENABLE_CONNECT_PROTOCOL:
      case hq::SettingId::H3_DATAGRAM:
      case hq::SettingId::H3_DATAGRAM_DRAFT_8:
      case hq::SettingId::H3_DATAGRAM_RFC:
      case hq::SettingId::ENABLE_WEBTRANSPORT:
        // only 0/1 are legal
        if (setting.second > 1) {
          return HTTP3::ErrorCode::HTTP_SETTINGS_ERROR;
        }
        break;
      default:
        continue; // ignore unknown settings
    }
    auto httpSettingId = hqToHttpSettingsId(setting.first);
    ingressSettings.setSetting(*httpSettingId, setting.second);
    settingsList.push_back(*ingressSettings.getSetting(*httpSettingId));
  }

  if (callback_) {
    callback_->onSettings(settingsList);
  }
  return folly::none;
}

ParseResult HQControlCodec::parseGoaway(Cursor& cursor,
                                        const FrameHeader& header) {
  quic::StreamId outStreamId;
  auto res = hq::parseGoaway(cursor, header, outStreamId);
  if (!res && callback_) {
    callback_->onGoaway(outStreamId, ErrorCode::NO_ERROR);
  }
  return res;
}

ParseResult HQControlCodec::parseMaxPushId(Cursor& cursor,
                                           const FrameHeader& header) {
  quic::StreamId outPushId;
  auto res = hq::parseMaxPushId(cursor, header, outPushId);
  return res;
}

ParseResult HQControlCodec::parsePriorityUpdate(Cursor& cursor,
                                                const FrameHeader& header) {
  HTTPCodec::StreamID prioritizedElement;
  HTTPPriority priorityUpdate;
  auto res = hq::parsePriorityUpdate(
      cursor, header, prioritizedElement, priorityUpdate);
  if (!res) {
    callback_->onPriority(folly::to<quic::StreamId>(prioritizedElement),
                          priorityUpdate);
  }
  return res;
}

ParseResult HQControlCodec::parsePushPriorityUpdate(Cursor& cursor,
                                                    const FrameHeader& header) {
  HTTPCodec::StreamID prioritizedElement;
  HTTPPriority priorityUpdate;
  auto res = hq::parsePriorityUpdate(
      cursor, header, prioritizedElement, priorityUpdate);
  if (!res) {
    callback_->onPushPriority(folly::to<PushId>(prioritizedElement),
                              priorityUpdate);
  }
  return res;
}

bool HQControlCodec::isWaitingToDrain() const {
  return (!doubleGoaway_ && !sentGoaway_) ||
         (doubleGoaway_ && sentGoaway_ && !sentFinalGoaway_);
}

uint64_t HQControlCodec::finalGoawayId() {
  if (transportDirection_ == TransportDirection::DOWNSTREAM) {
    return minUnseenStreamID_;
  } else {
    return minUnseenPushID_;
  }
}

size_t HQControlCodec::generateGoaway(
    folly::IOBufQueue& writeBuf,
    StreamID minUnseenId,
    ErrorCode statusCode,
    std::unique_ptr<folly::IOBuf> /*debugData*/) {
  if (sentFinalGoaway_) {
    return 0;
  }
  if (minUnseenId == HTTPCodec::MaxStreamID) {
    if (statusCode != ErrorCode::NO_ERROR || isWaitingToDrain()) {
      // Non-draining goaway, but the caller didn't know the ID
      // HQSession doesn't use this path now
      minUnseenId = finalGoawayId();
      sentFinalGoaway_ = true;
    } else {
      // Draining goaway
      minUnseenId = drainingId(transportDirection_);
    }
  } else {
    // Non-draining goaway, caller supplied ID
    sentFinalGoaway_ = true;
  }
  VLOG(4) << "generating GOAWAY minUnseenId=" << minUnseenId
          << " statusCode=" << uint32_t(statusCode);

  DCHECK_GE(egressGoawayAck_, minUnseenId);
  egressGoawayAck_ = minUnseenId;
  auto writeRes = hq::writeGoaway(writeBuf, minUnseenId);
  if (writeRes.hasError()) {
    LOG(FATAL) << "error writing goaway with minUnseenId=" << minUnseenId;
    return 0;
  }
  sentGoaway_ = true;
  return *writeRes;
}

size_t HQControlCodec::generateSettings(folly::IOBufQueue& writeBuf) {
  CHECK(!sentSettings_);
  sentSettings_ = true;
  std::deque<hq::SettingPair> settings;
  for (auto& setting : getEgressSettings()->getAllSettings()) {
    auto id = httpToHqSettingsId(setting.id);
    // unknown ids will return folly::none
    if (id) {
      switch (*id) {
        case hq::SettingId::HEADER_TABLE_SIZE:
        case hq::SettingId::MAX_HEADER_LIST_SIZE:
        case hq::SettingId::QPACK_BLOCKED_STREAMS:
        case hq::SettingId::ENABLE_CONNECT_PROTOCOL:
        case hq::SettingId::H3_DATAGRAM:
        case hq::SettingId::H3_DATAGRAM_DRAFT_8:
        case hq::SettingId::H3_DATAGRAM_RFC:
        case hq::SettingId::ENABLE_WEBTRANSPORT:
        case hq::SettingId::WEBTRANSPORT_MAX_SESSIONS:
          break;
      }
      settings.emplace_back(*id, (SettingValue)setting.value);
    }
  }
  // add a random setting from the greasing pool
  settings.emplace_back(
      static_cast<SettingId>(*getGreaseId(folly::Random::rand32(16))),
      static_cast<SettingValue>(0xFACEB00C));
  auto writeRes = writeSettings(writeBuf, settings);
  if (writeRes.hasError()) {
    LOG(FATAL) << "error writing settings frame";
    return 0;
  }
  return *writeRes;
}

size_t HQControlCodec::generatePriority(
    folly::IOBufQueue& /*writeBuf*/,
    StreamID /*stream*/,
    const HTTPMessage::HTTP2Priority& /*pri*/) {
  CHECK(false) << __func__
               << " deprecated draft. Use the other generatePriority API";
  return 0;
}

size_t HQControlCodec::generatePriority(folly::IOBufQueue& writeBuf,
                                        StreamID stream,
                                        HTTPPriority priority) {
  if (priority.urgency > quic::kDefaultMaxPriority) {
    LOG(ERROR) << "Attempt to generate invalid priority update with urgency="
               << (uint64_t)priority.urgency;
    return 0;
  }
  std::string updateString = folly::to<std::string>(
      "u=", priority.urgency, (priority.incremental ? ",i" : ""));
  auto writeRet = hq::writePriorityUpdate(writeBuf, stream, updateString);
  if (writeRet.hasError()) {
    LOG(ERROR) << "error writing priority update, stream=" << stream
               << ", priority=" << updateString;
    return 0;
  }
  return *writeRet;
}

size_t HQControlCodec::generatePushPriority(folly::IOBufQueue& writeBuf,
                                            StreamID pushId,
                                            HTTPPriority priority) {
  if (priority.urgency > quic::kDefaultMaxPriority) {
    LOG(ERROR)
        << "Attempt to generate invalid push priority update with urgency="
        << (uint64_t)priority.urgency;
    return 0;
  }
  std::string updateString = folly::to<std::string>(
      "u=", priority.urgency, (priority.incremental ? ",i" : ""));
  auto writeRet = hq::writePushPriorityUpdate(writeBuf, pushId, updateString);
  if (writeRet.hasError()) {
    LOG(ERROR) << "error writing push priority update, pushId=" << pushId
               << ", priority=" << updateString;
    return 0;
  }
  return *writeRet;
}

size_t HQControlCodec::addPriorityNodes(PriorityQueue& /*queue*/,
                                        folly::IOBufQueue& /*writeBuf*/,
                                        uint8_t /*maxLevel*/) {

  CHECK(false) << __func__ << " not implemented";
  return 0;
}

HTTPCodec::StreamID HQControlCodec::mapPriorityToDependency(
    uint8_t /*priority*/) const {
  CHECK(false) << __func__ << " not implemented";
  return 0;
}

}} // namespace proxygen::hq
