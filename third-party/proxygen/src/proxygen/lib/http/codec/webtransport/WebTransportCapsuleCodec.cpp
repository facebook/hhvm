/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/webtransport/WebTransportCapsuleCodec.h>

namespace proxygen {

folly::Expected<folly::Unit, CapsuleCodec::ErrorCode>
WebTransportCapsuleCodec::parseCapsule(folly::io::Cursor& cursor) {
  switch (curCapsuleType_) {
    case folly::to_underlying(CapsuleType::PADDING):
      return dispatchParsedFrame(parsePadding(cursor, curCapsuleLength_),
                                 callback_,
                                 &Callback::onPadding);
    case folly::to_underlying(CapsuleType::WT_RESET_STREAM):
      return dispatchParsedFrame(parseWTResetStream(cursor, curCapsuleLength_),
                                 callback_,
                                 &Callback::onResetStream);
    case folly::to_underlying(CapsuleType::WT_STOP_SENDING):
      return dispatchParsedFrame(parseWTStopSending(cursor, curCapsuleLength_),
                                 callback_,
                                 &Callback::onStopSending);
    case folly::to_underlying(CapsuleType::WT_STREAM):
    case folly::to_underlying(CapsuleType::WT_STREAM_WITH_FIN): {
      bool fin = curCapsuleType_ ==
                 folly::to_underlying(CapsuleType::WT_STREAM_WITH_FIN);
      return dispatchParsedFrame(parseWTStream(cursor, curCapsuleLength_, fin),
                                 callback_,
                                 &Callback::onStream);
    }
    case folly::to_underlying(CapsuleType::WT_MAX_DATA):
      return dispatchParsedFrame(parseWTMaxData(cursor, curCapsuleLength_),
                                 callback_,
                                 &Callback::onMaxData);
    case folly::to_underlying(CapsuleType::WT_MAX_STREAM_DATA):
      return dispatchParsedFrame(
          parseWTMaxStreamData(cursor, curCapsuleLength_),
          callback_,
          &Callback::onMaxStreamData);
    case folly::to_underlying(CapsuleType::WT_MAX_STREAMS_BIDI):
      return dispatchParsedFrame(parseWTMaxStreams(cursor, curCapsuleLength_),
                                 callback_,
                                 &Callback::onMaxStreamsBidi);
    case folly::to_underlying(CapsuleType::WT_MAX_STREAMS_UNI):
      return dispatchParsedFrame(parseWTMaxStreams(cursor, curCapsuleLength_),
                                 callback_,
                                 &Callback::onMaxStreamsUni);
    case folly::to_underlying(CapsuleType::WT_DATA_BLOCKED):
      return dispatchParsedFrame(parseWTDataBlocked(cursor, curCapsuleLength_),
                                 callback_,
                                 &Callback::onDataBlocked);
    case folly::to_underlying(CapsuleType::WT_STREAM_DATA_BLOCKED):
      return dispatchParsedFrame(
          parseWTStreamDataBlocked(cursor, curCapsuleLength_),
          callback_,
          &Callback::onStreamDataBlocked);
    case folly::to_underlying(CapsuleType::WT_STREAMS_BLOCKED_BIDI):
      return dispatchParsedFrame(
          parseWTStreamsBlocked(cursor, curCapsuleLength_),
          callback_,
          &Callback::onStreamsBlockedBidi);
    case folly::to_underlying(CapsuleType::WT_STREAMS_BLOCKED_UNI):
      return dispatchParsedFrame(
          parseWTStreamsBlocked(cursor, curCapsuleLength_),
          callback_,
          &Callback::onStreamsBlockedUni);
    case folly::to_underlying(CapsuleType::DATAGRAM):
      return dispatchParsedFrame(parseDatagram(cursor, curCapsuleLength_),
                                 callback_,
                                 &Callback::onDatagram);
    case folly::to_underlying(CapsuleType::CLOSE_WEBTRANSPORT_SESSION):
      return dispatchParsedFrame(
          parseCloseWebTransportSession(cursor, curCapsuleLength_),
          callback_,
          &Callback::onCloseSession);
    case folly::to_underlying(CapsuleType::DRAIN_WEBTRANSPORT_SESSION):
      return dispatchParsedFrame(
          parseDrainWebTransportSession(curCapsuleLength_),
          callback_,
          &Callback::onDrainSession);
    default:
      return folly::makeUnexpected(CapsuleCodec::ErrorCode::PARSE_ERROR);
  }
}
} // namespace proxygen
