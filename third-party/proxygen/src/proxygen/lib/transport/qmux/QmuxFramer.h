/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>

#include <folly/Expected.h>
#include <folly/Optional.h>
#include <folly/Utility.h>
#include <folly/io/IOBuf.h>
#include <proxygen/lib/http/codec/CapsuleCodec.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportFramer.h>

namespace proxygen::qmux {

// Shorthand for QmuxFrameType wire values.
constexpr uint64_t qval(QmuxFrameType t) {
  return folly::to_underlying(t);
}

// STREAM frame flag bits (lower 3 bits of type 0x08..0x0f)
constexpr uint8_t kStreamFlagFin = 0x01;
constexpr uint8_t kStreamFlagLen = 0x02;
constexpr uint8_t kStreamFlagOff = 0x04;

constexpr uint64_t kDefaultMaxRecordSize = 16382;

// QUIC transport parameter IDs (RFC 9000 Section 18.2)
constexpr uint64_t kTpMaxIdleTimeout = 0x01;
constexpr uint64_t kTpInitialMaxData = 0x04;
constexpr uint64_t kTpInitialMaxStreamDataBidiLocal = 0x05;
constexpr uint64_t kTpInitialMaxStreamDataBidiRemote = 0x06;
constexpr uint64_t kTpInitialMaxStreamDataUni = 0x07;
constexpr uint64_t kTpInitialMaxStreamsBidi = 0x08;
constexpr uint64_t kTpInitialMaxStreamsUni = 0x09;
constexpr uint64_t kTpMaxRecordSize = 0x0571c59429cd0845;
// Datagram extension TP
constexpr uint64_t kTpMaxDatagramFrameSize = 0x20;

//////// QMUX-Specific Structs ////////

struct QxConnectionClose {
  uint64_t errorCode{0};
  uint64_t frameType{0};  // only meaningful for type 0x1c (transport-level)
  bool isAppError{false}; // true for type 0x1d
  std::string reasonPhrase;
};

struct QxTransportParams {
  uint64_t maxIdleTimeout{0};
  uint64_t initialMaxData{0};
  uint64_t initialMaxStreamDataBidiLocal{0};
  uint64_t initialMaxStreamDataBidiRemote{0};
  uint64_t initialMaxStreamDataUni{0};
  uint64_t initialMaxStreamsBidi{0};
  uint64_t initialMaxStreamsUni{0};
  uint64_t maxRecordSize{kDefaultMaxRecordSize};
  folly::Optional<uint64_t> maxDatagramFrameSize;
};

struct QxPing {
  uint64_t sequenceNumber{0};
};

//////// Error Code ////////

using QmuxErrorCode = CapsuleCodec::ErrorCode;

//////// QMUX-Specific Parse Functions ////////

// Called when the STREAM frame has an OFF flag. Returns true if the offset
// is valid (i.e., contiguous with prior data on the same stream).
// Arguments: streamId, offset
using OffsetValidator = std::function<bool(uint64_t streamId, uint64_t offset)>;

folly::Expected<WTStreamCapsule, QmuxErrorCode> parseStream(
    folly::io::Cursor& cursor,
    uint8_t flags,
    size_t length,
    const OffsetValidator& validateOffset = nullptr);

folly::Expected<QxConnectionClose, QmuxErrorCode> parseConnectionClose(
    folly::io::Cursor& cursor, size_t length, bool isAppError);

folly::Expected<QxTransportParams, QmuxErrorCode> parseTransportParams(
    folly::io::Cursor& cursor, size_t length);

folly::Expected<QxPing, QmuxErrorCode> parsePing(folly::io::Cursor& cursor,
                                                 size_t length);

//////// QMUX-Specific Write Functions ////////

using proxygen::WriteResult;

WriteResult writeRecord(folly::IOBufQueue& queue,
                        std::unique_ptr<folly::IOBuf> frames);

WriteResult writeConnectionClose(folly::IOBufQueue& queue,
                                 const QxConnectionClose& frame);

WriteResult writeTransportParams(folly::IOBufQueue& queue,
                                 const QxTransportParams& params);

WriteResult writePing(folly::IOBufQueue& queue, const QxPing& ping);

WriteResult writePong(folly::IOBufQueue& queue, const QxPing& pong);

} // namespace proxygen::qmux
