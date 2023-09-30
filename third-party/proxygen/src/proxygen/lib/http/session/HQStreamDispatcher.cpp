/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HQStreamDispatcher.h>

using namespace proxygen;

HQStreamDispatcherBase::HQStreamDispatcherBase(
    CallbackBase& callback, proxygen::TransportDirection direction)
    : callback_(callback), direction_(direction) {
}

void HQStreamDispatcherBase::peekError(quic::StreamId id,
                                       quic::QuicError error) noexcept {
  VLOG(4) << __func__ << ": peekError streamID=" << id << " error: " << error;

  switch (error.code.type()) {
    case quic::QuicErrorCode::Type::ApplicationErrorCode: {
      auto errorCode =
          static_cast<HTTP3::ErrorCode>(*error.code.asApplicationErrorCode());
      VLOG(4) << "peekError: QUIC Application Error: " << toString(errorCode)
              << " streamID=" << id;
      break;
    }
    case quic::QuicErrorCode::Type::LocalErrorCode: {
      quic::LocalErrorCode& errorCode = *error.code.asLocalErrorCode();
      VLOG(4) << "peekError: QUIC Local Error: " << errorCode
              << " streamID=" << id;
      break;
    }
    case quic::QuicErrorCode::Type::TransportErrorCode: {
      quic::TransportErrorCode& errorCode = *error.code.asTransportErrorCode();
      VLOG(4) << "peekError: QUIC Transport Error: " << errorCode
              << " streamID=" << id;
      break;
    }
  }
}

void HQStreamDispatcherBase::onDataAvailable(
    quic::StreamId id, const CallbackBase::PeekData& peekData) noexcept {
  if (peekData.empty()) {
    return;
  }

  auto& peekFirst = peekData.front();
  // if not at offset 0, ignore
  if (peekFirst.offset != 0) {
    return;
  }

  auto maybeClearPeekCallback = folly::makeGuard([&] {
    if (peekFirst.eof) {
      // Chunk offset 0 with EOF means we must dispatch, or clear the cb
      VLOG(4) << "Undispatchable stream (EOF before preface complete)";
      callback_.rejectStream(releaseOwnership(id));
    }
  });
  // empty buffer, just EOF
  auto dataBuf = peekFirst.data.front();
  if (!dataBuf) {
    return;
  }

  // Look for a stream preface in the first read buffer
  VLOG(4) << "Attempting peek dispatch stream=" << id
          << " len=" << dataBuf->computeChainDataLength();
  folly::io::Cursor cursor(dataBuf);
  auto preface = quic::decodeQuicInteger(cursor);
  if (!preface) {
    return;
  }

  auto result = handleStream(id, cursor, preface->first, preface->second);
  if (result != HandleStreamResult::PENDING) {
    maybeClearPeekCallback.dismiss();
    if (result == HandleStreamResult::REJECT) {
      callback_.rejectStream(releaseOwnership(id));
    }
  }
}

HQStreamDispatcherBase::HandleStreamResult HQUniStreamDispatcher::handleStream(
    quic::StreamId id,
    folly::io::Cursor& cursor,
    uint64_t preface,
    size_t consumed) {
  auto type = callback_.parseUniStreamPreface(preface);

  if (!type) {
    // Failed to identify the preface,
    return HandleStreamResult::REJECT;
  }

  switch (type.value()) {
    case hq::UnidirectionalStreamType::CONTROL:
    case hq::UnidirectionalStreamType::QPACK_ENCODER:
    case hq::UnidirectionalStreamType::QPACK_DECODER: {
      // This is a control stream, and it needs a read callback
      // Pass ownership back to the callback
      callback_.dispatchControlStream(
          releaseOwnership(id), type.value(), consumed);
      return HandleStreamResult::DISPATCHED;
    }
    case hq::UnidirectionalStreamType::PUSH: {
      // ingress push streams are not allowed on the server
      if (direction_ == proxygen::TransportDirection::DOWNSTREAM) {
        return HandleStreamResult::REJECT;
      }
      // Try to read the push id from the stream
      auto pushId = quic::decodeQuicInteger(cursor);
      // If successfully read the push id, call callback
      // which will reassign the peek callback
      // Otherwise, continue using this callback
      if (pushId) {
        consumed += pushId->second;
        callback_.dispatchPushStream(
            releaseOwnership(id), pushId->first, consumed);
        return HandleStreamResult::DISPATCHED;
      } else {
        return HandleStreamResult::PENDING;
      }
    }
    case hq::UnidirectionalStreamType::WEBTRANSPORT: {
      // Try to read the session id from the stream
      auto sessionID = quic::decodeQuicInteger(cursor);
      // If successfully read the session id, call sink
      // which will reassign the peek callback
      // Otherwise, continue using this callback
      if (sessionID) {
        consumed += sessionID->second;
        callback_.dispatchUniWTStream(
            releaseOwnership(id), sessionID->first, consumed);
        return HandleStreamResult::DISPATCHED;
      } else {
        return HandleStreamResult::PENDING;
      }
    }
    case hq::UnidirectionalStreamType::GREASE:
      VLOG(4) << "Hey, a grease stream id=" << id;
      break;
    default:
      LOG(ERROR) << "Unrecognized type=" << folly::to_underlying(*type);
  }
  return HandleStreamResult::REJECT;
}

HQStreamDispatcherBase::HandleStreamResult HQBidiStreamDispatcher::handleStream(
    quic::StreamId id,
    folly::io::Cursor& cursor,
    uint64_t preface,
    size_t consumed) {
  auto type = callback_.parseBidiStreamPreface(preface);

  if (!type) {
    // Failed to identify the preface,
    return HandleStreamResult::REJECT;
  }

  switch (type.value()) {
    case hq::BidirectionalStreamType::REQUEST:
      callback_.dispatchRequestStream(releaseOwnership(id));
      return HandleStreamResult::DISPATCHED;
    case hq::BidirectionalStreamType::WEBTRANSPORT: {
      // Try to read the session id from the stream
      auto sessionID = quic::decodeQuicInteger(cursor);
      // If successfully read the session id, call sink
      // which will reassign the peek callback
      // Otherwise, continue using this callback
      if (sessionID) {
        consumed += sessionID->second;
        callback_.dispatchBidiWTStream(
            releaseOwnership(id), sessionID->first, consumed);
        return HandleStreamResult::DISPATCHED;
      } else {
        return HandleStreamResult::PENDING;
      }
    }
    default: {
      LOG(ERROR) << "Unrecognized type=" << static_cast<uint64_t>(type.value());
    }
  }
  return HandleStreamResult::REJECT;
}
