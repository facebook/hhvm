/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HQUnidirectionalCallbacks.h>

using namespace proxygen;

HQUnidirStreamDispatcher::HQUnidirStreamDispatcher(
    HQUnidirStreamDispatcher::Callback& sink,
    proxygen::TransportDirection direction)
    : controlStreamCallback_(std::make_unique<ControlCallback>(sink)),
      sink_(sink),
      direction_(direction) {
}

void HQUnidirStreamDispatcher::peekError(quic::StreamId id,
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

void HQUnidirStreamDispatcher::onDataAvailable(
    quic::StreamId id, const Callback::PeekData& peekData) noexcept {
  if (peekData.empty()) {
    return;
  }

  auto& peekFirst = peekData.front();
  // if not at offset 0, ignore
  if (peekFirst.offset != 0) {
    return;
  }

  // empty buffer, just EOF
  auto dataBuf = peekFirst.data.front();
  if (!dataBuf) {
    return;
  }

  // Look for a stream preface in the first read buffer
  folly::io::Cursor cursor(dataBuf);
  auto preface = quic::decodeQuicInteger(cursor);
  if (!preface) {
    return;
  }

  auto consumed = preface->second;
  auto type = sink_.parseStreamPreface(preface->first);

  if (!type) {
    // Failed to identify the preface,
    // release ownership and signal error
    sink_.rejectStream(releaseOwnership(id));
    return;
  }

  switch (type.value()) {
    case hq::UnidirectionalStreamType::H1Q_CONTROL:
    case hq::UnidirectionalStreamType::CONTROL:
    case hq::UnidirectionalStreamType::QPACK_ENCODER:
    case hq::UnidirectionalStreamType::QPACK_DECODER: {
      // This is a control stream, and it needs a read callback
      // Pass ownership back to the sink
      sink_.assignReadCallback(releaseOwnership(id),
                               type.value(),
                               consumed,
                               controlStreamCallback());
      return;
    }
    case hq::UnidirectionalStreamType::PUSH: {
      // ingress push streams are not allowed on the server
      if (direction_ == proxygen::TransportDirection::DOWNSTREAM) {
        sink_.rejectStream(releaseOwnership(id));
        return;
      }
      // Try to read the push id from the stream
      auto pushId = quic::decodeQuicInteger(cursor);
      // If successfully read the push id, call sink
      // which will reassign the peek callback
      // Otherwise, continue using this callback
      if (pushId) {
        consumed += pushId->second;
        sink_.onNewPushStream(releaseOwnership(id), pushId->first, consumed);
      }
      return;
    }
    default: {
      LOG(ERROR) << "Unrecognized type=" << static_cast<uint64_t>(type.value());
    }
  }
}

quic::QuicSocket::ReadCallback*
HQUnidirStreamDispatcher::controlStreamCallback() const {
  return controlStreamCallback_.get();
}

// Control stream callback implementation
void HQUnidirStreamDispatcher::ControlCallback::readError(
    quic::StreamId id,
    HQUnidirStreamDispatcher::Callback::ReadError error) noexcept {
  sink_.controlStreamReadError(id, error);
}

void HQUnidirStreamDispatcher::ControlCallback::readAvailable(
    quic::StreamId id) noexcept {
  sink_.controlStreamReadAvailable(id);
}
