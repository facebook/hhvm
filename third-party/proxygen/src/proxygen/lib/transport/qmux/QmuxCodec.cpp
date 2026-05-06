/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/qmux/QmuxCodec.h>

#include <quic/folly_utils/Utils.h>

#define QMUX_REPORT_AND_RETURN(errCode)        \
  do {                                         \
    connError_ = (errCode);                    \
    callback_->onConnectionError(*connError_); \
    return;                                    \
  } while (0)

namespace proxygen::qmux {

QmuxCodec::QmuxCodec(Callback* cb, OffsetValidator offsetValidator)
    : callback_(cb), offsetValidator_(std::move(offsetValidator)) {
}

void QmuxCodec::setMaxRecordSize(uint64_t maxRecordSize) {
  maxRecordSize_ = maxRecordSize;
}

namespace {
bool isAllowedFrameType(uint64_t type) {
  switch (type) {
    case qval(QmuxFrameType::PADDING):
    case qval(QmuxFrameType::RESET_STREAM):
    case qval(QmuxFrameType::STOP_SENDING):
    case qval(QmuxFrameType::MAX_DATA):
    case qval(QmuxFrameType::MAX_STREAM_DATA):
    case qval(QmuxFrameType::MAX_STREAMS_BIDI):
    case qval(QmuxFrameType::MAX_STREAMS_UNI):
    case qval(QmuxFrameType::DATA_BLOCKED):
    case qval(QmuxFrameType::STREAM_DATA_BLOCKED):
    case qval(QmuxFrameType::STREAMS_BLOCKED_BIDI):
    case qval(QmuxFrameType::STREAMS_BLOCKED_UNI):
    case qval(QmuxFrameType::CONNECTION_CLOSE):
    case qval(QmuxFrameType::CONNECTION_CLOSE_APP):
    case qval(QmuxFrameType::DATAGRAM_NO_LEN):
    case qval(QmuxFrameType::DATAGRAM):
    case qval(QmuxFrameType::QX_TRANSPORT_PARAMS):
    case qval(QmuxFrameType::QX_PING):
    case qval(QmuxFrameType::QX_PONG):
      return true;
    default:
      return (type >= qval(QmuxFrameType::STREAM_BASE) &&
              type <= (qval(QmuxFrameType::STREAM_BASE) + 0x07));
  }
}

bool hasLengthField(uint64_t type) {
  return type == qval(QmuxFrameType::QX_TRANSPORT_PARAMS) ||
         type == qval(QmuxFrameType::DATAGRAM);
}
} // namespace

void QmuxCodec::onIngress(std::unique_ptr<folly::IOBuf> data) {
  ingress_.append(std::move(data)); // ok if nullptr

  while (!connError_ && !ingress_.empty()) {
    auto available = ingress_.chainLength();
    folly::io::Cursor cursor(ingress_.front());

    // Outer loop: parse record header (Size varint)
    auto sizeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
    if (!sizeOpt) {
      break; // need more data for Size varint
    }
    uint64_t recordSize = sizeOpt->first;
    size_t sizeFieldLen = sizeOpt->second;

    if (recordSize > maxRecordSize_) {
      QMUX_REPORT_AND_RETURN(QmuxErrorCode::FRAME_ENCODING_ERROR);
    }

    if (available - sizeFieldLen < recordSize) {
      break; // need more data for complete record
    }

    // We have a complete record. Parse frames within it.
    size_t recordRemaining = recordSize;

    while (recordRemaining > 0 && !connError_) {
      auto typeOpt =
          quic::follyutils::decodeQuicInteger(cursor, recordRemaining);
      if (!typeOpt) {
        QMUX_REPORT_AND_RETURN(QmuxErrorCode::FRAME_ENCODING_ERROR);
      }
      uint64_t frameType = typeOpt->first;
      size_t typeSize = typeOpt->second;
      recordRemaining -= typeSize;

      if ((!receivedTransportParams_ &&
           frameType != qval(QmuxFrameType::QX_TRANSPORT_PARAMS)) ||
          (receivedTransportParams_ &&
           frameType == qval(QmuxFrameType::QX_TRANSPORT_PARAMS))) {
        connError_ = QmuxErrorCode::TRANSPORT_PARAMETER_ERROR;
        callback_->onConnectionError(QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        return;
      }

      if (!isAllowedFrameType(frameType)) {
        QMUX_REPORT_AND_RETURN(QmuxErrorCode::FRAME_ENCODING_ERROR);
      }

      if (frameType == qval(QmuxFrameType::PADDING)) {
        continue;
      }

      // Track cursor position to compute consumed bytes
      auto cursorBefore = cursor;

      if (frameType >= qval(QmuxFrameType::STREAM_BASE) &&
          frameType <= (qval(QmuxFrameType::STREAM_BASE) + 0x07)) {
        uint8_t flags = static_cast<uint8_t>(frameType & 0x07);
        auto result =
            parseStream(cursor, flags, recordRemaining, offsetValidator_);
        if (!result) {
          connError_ = result.error();
          callback_->onConnectionError(result.error());
          return;
        }
        recordRemaining -= (cursor - cursorBefore);
        callback_->onStream(std::move(*result));
        continue;
      }

      if (hasLengthField(frameType)) {
        auto lenOpt =
            quic::follyutils::decodeQuicInteger(cursor, recordRemaining);
        if (!lenOpt) {
          QMUX_REPORT_AND_RETURN(QmuxErrorCode::FRAME_ENCODING_ERROR);
        }
        recordRemaining -= lenOpt->second;
        size_t payloadLen = lenOpt->first;

        if (payloadLen > recordRemaining) {
          QMUX_REPORT_AND_RETURN(QmuxErrorCode::FRAME_ENCODING_ERROR);
        }

        if (frameType == qval(QmuxFrameType::QX_TRANSPORT_PARAMS)) {
          auto result = parseTransportParams(cursor, payloadLen);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= payloadLen;
          receivedTransportParams_ = true;
          callback_->onTransportParameters(std::move(*result));
        } else if (frameType == qval(QmuxFrameType::DATAGRAM)) {
          auto result = proxygen::parseDatagram(cursor, payloadLen);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= payloadLen;
          callback_->onDatagram(std::move(*result));
        }
        continue;
      }

      if (frameType == qval(QmuxFrameType::DATAGRAM_NO_LEN)) {
        auto result = proxygen::parseDatagram(cursor, recordRemaining);
        if (!result) {
          connError_ = result.error();
          callback_->onConnectionError(result.error());
          return;
        }
        recordRemaining = 0;
        callback_->onDatagram(std::move(*result));
        continue;
      }

      switch (frameType) {
        case qval(QmuxFrameType::RESET_STREAM): {
          auto result = proxygen::parseWTResetStream(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onResetStream(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::STOP_SENDING): {
          auto result = proxygen::parseWTStopSending(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onStopSending(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::MAX_DATA): {
          auto result = proxygen::parseWTMaxData(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onMaxData(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::MAX_STREAM_DATA): {
          auto result = proxygen::parseWTMaxStreamData(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onMaxStreamData(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::MAX_STREAMS_BIDI): {
          auto result = proxygen::parseWTMaxStreams(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onMaxStreamsBidi(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::MAX_STREAMS_UNI): {
          auto result = proxygen::parseWTMaxStreams(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onMaxStreamsUni(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::DATA_BLOCKED): {
          auto result = proxygen::parseWTDataBlocked(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onDataBlocked(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::STREAM_DATA_BLOCKED): {
          auto result =
              proxygen::parseWTStreamDataBlocked(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onStreamDataBlocked(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::STREAMS_BLOCKED_BIDI): {
          auto result =
              proxygen::parseWTStreamsBlocked(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onStreamsBlockedBidi(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::STREAMS_BLOCKED_UNI): {
          auto result =
              proxygen::parseWTStreamsBlocked(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onStreamsBlockedUni(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::QX_PING): {
          auto result = parsePing(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onPing(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::QX_PONG): {
          auto result = parsePing(cursor, recordRemaining);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onPong(std::move(*result));
          continue;
        }
        case qval(QmuxFrameType::CONNECTION_CLOSE):
        case qval(QmuxFrameType::CONNECTION_CLOSE_APP): {
          bool isApp = (frameType == qval(QmuxFrameType::CONNECTION_CLOSE_APP));
          auto result = parseConnectionClose(cursor, recordRemaining, isApp);
          if (!result) {
            connError_ = result.error();
            callback_->onConnectionError(result.error());
            return;
          }
          recordRemaining -= (cursor - cursorBefore);
          callback_->onConnectionClose(std::move(*result));
          continue;
        }
        default:
          QMUX_REPORT_AND_RETURN(QmuxErrorCode::FRAME_ENCODING_ERROR);
      }
    }

    // Trim the entire record (Size field + Frames) from ingress
    if (!connError_) {
      ingress_.trimStart(sizeFieldLen + recordSize);
    }
  }
}

} // namespace proxygen::qmux
