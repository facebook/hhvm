/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/qmux/QmuxFramer.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <proxygen/lib/http/codec/VarintUtils.h>
#include <quic/codec/QuicInteger.h>
#include <quic/folly_utils/Utils.h>

using proxygen::writeVarint;

namespace {
// Set of prohibited QUIC v1 transport parameter IDs
bool isProhibitedTransportParam(uint64_t paramId) {
  switch (paramId) {
    case 0x00: // original_destination_connection_id
    case 0x02: // stateless_reset_token
    case 0x03: // max_udp_payload_size
    case 0x0a: // ack_delay_exponent
    case 0x0b: // max_ack_delay
    case 0x0c: // disable_active_migration
    case 0x0d: // preferred_address
    case 0x0e: // active_connection_id_limit
    case 0x0f: // initial_source_connection_id
    case 0x10: // retry_source_connection_id
      return true;
    default:
      return false;
  }
}

} // namespace

namespace proxygen::qmux {

//////// QMUX-Specific Parse Implementations ////////

folly::Expected<WTStreamCapsule, QmuxErrorCode> parseStream(
    folly::io::Cursor& cursor,
    uint8_t flags,
    size_t length,
    const OffsetValidator& validateOffset) {
  WTStreamCapsule frame;

  bool fin = flags & kStreamFlagFin;
  bool hasLen = flags & kStreamFlagLen;
  bool hasOff = flags & kStreamFlagOff;

  // Parse Stream ID
  auto streamIdOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!streamIdOpt) {
    return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
  }
  length -= streamIdOpt->second;
  frame.streamId = streamIdOpt->first;

  // Parse Offset (if OFF flag set) and validate via callback
  if (hasOff) {
    auto offsetOpt = quic::follyutils::decodeQuicInteger(cursor, length);
    if (!offsetOpt) {
      return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
    }
    length -= offsetOpt->second;
    if (validateOffset && !validateOffset(frame.streamId, offsetOpt->first)) {
      return folly::makeUnexpected(QmuxErrorCode::PROTOCOL_VIOLATION);
    }
  }

  // Parse data
  uint64_t dataLen = 0;
  if (hasLen) {
    auto lenOpt = quic::follyutils::decodeQuicInteger(cursor, length);
    if (!lenOpt) {
      return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
    }
    length -= lenOpt->second;
    dataLen = lenOpt->first;
  } else {
    // Without LEN, data extends to the end of the frame
    dataLen = length;
  }

  if (dataLen > length) {
    return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
  }

  if (dataLen > 0) {
    if (!cursor.canAdvance(dataLen)) {
      return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
    }
    cursor.cloneAtMost(frame.streamData, dataLen);
  }
  length -= dataLen;

  frame.fin = fin;
  return frame;
}

folly::Expected<QxConnectionClose, QmuxErrorCode> parseConnectionClose(
    folly::io::Cursor& cursor, size_t length, bool isAppError) {
  QxConnectionClose frame;
  frame.isAppError = isAppError;

  auto errorCodeOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!errorCodeOpt) {
    return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
  }
  length -= errorCodeOpt->second;
  frame.errorCode = errorCodeOpt->first;

  if (!isAppError) {
    auto frameTypeOpt = quic::follyutils::decodeQuicInteger(cursor, length);
    if (!frameTypeOpt) {
      return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
    }
    length -= frameTypeOpt->second;
    frame.frameType = frameTypeOpt->first;
  }

  auto reasonLenOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!reasonLenOpt) {
    return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
  }
  length -= reasonLenOpt->second;
  uint64_t reasonLen = reasonLenOpt->first;

  if (reasonLen > length) {
    return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
  }
  if (reasonLen > 0) {
    if (!cursor.canAdvance(reasonLen)) {
      return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
    }
    frame.reasonPhrase.resize(reasonLen);
    cursor.pull(frame.reasonPhrase.data(), reasonLen);
  }
  length -= reasonLen;

  return frame;
}

folly::Expected<QxTransportParams, QmuxErrorCode> parseTransportParams(
    folly::io::Cursor& cursor, size_t length) {
  QxTransportParams params;

  while (length > 0) {
    auto paramIdOpt = quic::follyutils::decodeQuicInteger(cursor, length);
    if (!paramIdOpt) {
      return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
    }
    length -= paramIdOpt->second;
    uint64_t paramId = paramIdOpt->first;

    auto paramLenOpt = quic::follyutils::decodeQuicInteger(cursor, length);
    if (!paramLenOpt) {
      return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
    }
    length -= paramLenOpt->second;
    uint64_t paramLen = paramLenOpt->first;

    if (paramLen > length) {
      return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
    }

    if (isProhibitedTransportParam(paramId)) {
      return folly::makeUnexpected(QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
    }

    // Decode known params (value is a varint within paramLen bytes)
    auto decodeVarintParam = [&]() -> folly::Optional<uint64_t> {
      auto valOpt = quic::follyutils::decodeQuicInteger(cursor, paramLen);
      if (!valOpt || valOpt->second != paramLen) {
        return folly::none;
      }
      return valOpt->first;
    };

    switch (paramId) {
      case kTpMaxIdleTimeout: {
        auto val = decodeVarintParam();
        if (!val) {
          return folly::makeUnexpected(
              QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        }
        params.maxIdleTimeout = *val;
        break;
      }
      case kTpInitialMaxData: {
        auto val = decodeVarintParam();
        if (!val) {
          return folly::makeUnexpected(
              QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        }
        params.initialMaxData = *val;
        break;
      }
      case kTpInitialMaxStreamDataBidiLocal: {
        auto val = decodeVarintParam();
        if (!val) {
          return folly::makeUnexpected(
              QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        }
        params.initialMaxStreamDataBidiLocal = *val;
        break;
      }
      case kTpInitialMaxStreamDataBidiRemote: {
        auto val = decodeVarintParam();
        if (!val) {
          return folly::makeUnexpected(
              QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        }
        params.initialMaxStreamDataBidiRemote = *val;
        break;
      }
      case kTpInitialMaxStreamDataUni: {
        auto val = decodeVarintParam();
        if (!val) {
          return folly::makeUnexpected(
              QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        }
        params.initialMaxStreamDataUni = *val;
        break;
      }
      case kTpInitialMaxStreamsBidi: {
        auto val = decodeVarintParam();
        if (!val) {
          return folly::makeUnexpected(
              QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        }
        params.initialMaxStreamsBidi = *val;
        break;
      }
      case kTpInitialMaxStreamsUni: {
        auto val = decodeVarintParam();
        if (!val) {
          return folly::makeUnexpected(
              QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        }
        params.initialMaxStreamsUni = *val;
        break;
      }
      case kTpMaxRecordSize: {
        auto val = decodeVarintParam();
        if (!val) {
          return folly::makeUnexpected(
              QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        }
        if (*val < kDefaultMaxRecordSize) {
          return folly::makeUnexpected(
              QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        }
        params.maxRecordSize = *val;
        break;
      }
      case kTpMaxDatagramFrameSize: {
        auto val = decodeVarintParam();
        if (!val) {
          return folly::makeUnexpected(
              QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
        }
        params.maxDatagramFrameSize = *val;
        break;
      }
      default:
        // Unknown parameter - skip
        cursor.skip(paramLen);
        break;
    }
    length -= paramLen;
  }
  return params;
}

folly::Expected<QxPing, QmuxErrorCode> parsePing(folly::io::Cursor& cursor,
                                                 size_t length) {
  QxPing ping;
  auto seqOpt = quic::follyutils::decodeQuicInteger(cursor, length);
  if (!seqOpt) {
    return folly::makeUnexpected(QmuxErrorCode::PARSE_UNDERFLOW);
  }
  length -= seqOpt->second;
  ping.sequenceNumber = seqOpt->first;
  return ping;
}

//////// QMUX-Specific Write Implementations ////////

WriteResult writeRecord(folly::IOBufQueue& queue,
                        std::unique_ptr<folly::IOBuf> frames) {
  CHECK(frames) << "frames must be non-null";
  size_t size = 0;
  bool error = false;
  auto framesLen = frames->computeChainDataLength();
  writeVarint(queue, framesLen, size, error);
  queue.append(std::move(frames));
  size += framesLen;
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeConnectionClose(folly::IOBufQueue& queue,
                                 const QxConnectionClose& frame) {
  size_t size = 0;
  bool error = false;
  auto type = frame.isAppError ? qval(QmuxFrameType::CONNECTION_CLOSE_APP)
                               : qval(QmuxFrameType::CONNECTION_CLOSE);
  writeVarint(queue, type, size, error);
  writeVarint(queue, frame.errorCode, size, error);
  if (!frame.isAppError) {
    writeVarint(queue, frame.frameType, size, error);
  }
  writeVarint(queue, frame.reasonPhrase.size(), size, error);
  if (!frame.reasonPhrase.empty()) {
    queue.append(folly::IOBuf::copyBuffer(frame.reasonPhrase.data(),
                                          frame.reasonPhrase.size()));
    size += frame.reasonPhrase.size();
  }
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writeTransportParams(folly::IOBufQueue& queue,
                                 const QxTransportParams& params) {
  size_t size = 0;
  bool error = false;

  // Build the payload into a temporary buffer to get its length
  folly::IOBufQueue payload{folly::IOBufQueue::cacheChainLength()};
  size_t payloadSize = 0;

  auto writeParam = [&](uint64_t paramId, uint64_t value) {
    auto valSize = quic::getQuicIntegerSize(value);
    if (!valSize) {
      error = true;
      return;
    }
    writeVarint(payload, paramId, payloadSize, error);
    writeVarint(payload, *valSize, payloadSize, error);
    writeVarint(payload, value, payloadSize, error);
  };

  if (params.maxIdleTimeout > 0) {
    writeParam(kTpMaxIdleTimeout, params.maxIdleTimeout);
  }
  if (params.initialMaxData > 0) {
    writeParam(kTpInitialMaxData, params.initialMaxData);
  }
  if (params.initialMaxStreamDataBidiLocal > 0) {
    writeParam(kTpInitialMaxStreamDataBidiLocal,
               params.initialMaxStreamDataBidiLocal);
  }
  if (params.initialMaxStreamDataBidiRemote > 0) {
    writeParam(kTpInitialMaxStreamDataBidiRemote,
               params.initialMaxStreamDataBidiRemote);
  }
  if (params.initialMaxStreamDataUni > 0) {
    writeParam(kTpInitialMaxStreamDataUni, params.initialMaxStreamDataUni);
  }
  if (params.initialMaxStreamsBidi > 0) {
    writeParam(kTpInitialMaxStreamsBidi, params.initialMaxStreamsBidi);
  }
  if (params.initialMaxStreamsUni > 0) {
    writeParam(kTpInitialMaxStreamsUni, params.initialMaxStreamsUni);
  }
  if (params.maxRecordSize != kDefaultMaxRecordSize) {
    writeParam(kTpMaxRecordSize, params.maxRecordSize);
  }
  if (params.maxDatagramFrameSize.hasValue()) {
    writeParam(kTpMaxDatagramFrameSize, *params.maxDatagramFrameSize);
  }

  if (error) {
    return std::nullopt;
  }

  // Write type + length + payload
  writeVarint(queue, qval(QmuxFrameType::QX_TRANSPORT_PARAMS), size, error);
  writeVarint(queue, payloadSize, size, error);
  queue.append(payload.move());
  size += payloadSize;

  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writePing(folly::IOBufQueue& queue, const QxPing& ping) {
  size_t size = 0;
  bool error = false;
  writeVarint(queue, qval(QmuxFrameType::QX_PING), size, error);
  writeVarint(queue, ping.sequenceNumber, size, error);
  return error ? std::nullopt : std::make_optional(size);
}

WriteResult writePong(folly::IOBufQueue& queue, const QxPing& pong) {
  size_t size = 0;
  bool error = false;
  writeVarint(queue, qval(QmuxFrameType::QX_PONG), size, error);
  writeVarint(queue, pong.sequenceNumber, size, error);
  return error ? std::nullopt : std::make_optional(size);
}

} // namespace proxygen::qmux
