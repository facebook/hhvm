/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/CapsuleCodec.h>

namespace proxygen {

//////// Protocol Selection ////////

enum class FrameProtocol : uint8_t {
  WT_CAPSULE, // WebTransport over HTTP capsules (draft-ietf-webtrans-http)
  QMUX,       // QMUX framing (draft-ietf-quic-qmux)
};

//////// Types ////////

enum class CapsuleType : uint32_t {
  PADDING = 0x190B4D38,
  WT_RESET_STREAM = 0x190B4D39,
  WT_STOP_SENDING = 0x190B4D3A,
  WT_STREAM = 0x190B4D3B,
  WT_STREAM_WITH_FIN = 0x190B4D3C,
  WT_MAX_DATA = 0x190B4D3D,
  WT_MAX_STREAM_DATA = 0x190B4D3E,
  WT_MAX_STREAMS_BIDI = 0x190B4D3F,
  WT_MAX_STREAMS_UNI = 0x190B4D40,
  WT_DATA_BLOCKED = 0x190B4D41,
  WT_STREAM_DATA_BLOCKED = 0x190B4D42,
  WT_STREAMS_BLOCKED_BIDI = 0x190B4D43,
  WT_STREAMS_BLOCKED_UNI = 0x190B4D44,
  DATAGRAM = 0x00,
  CLOSE_WEBTRANSPORT_SESSION = 0x190B4D45,
  DRAIN_WEBTRANSPORT_SESSION = 0x190B4D46
};

// QMUX frame types from draft-ietf-quic-qmux, matching QUIC frame type values.
// Names match CapsuleType (minus WT_ prefix) for use with the WT_QMUX macro.
enum class QmuxFrameType : uint64_t {
  PADDING = 0x00,
  RESET_STREAM = 0x04,
  STOP_SENDING = 0x05,
  STREAM_BASE = 0x08,     // 0x08..0x0f with flag bits
  STREAM = 0x0a,          // STREAM_BASE | LEN
  STREAM_WITH_FIN = 0x0b, // STREAM_BASE | LEN | FIN
  MAX_DATA = 0x10,
  MAX_STREAM_DATA = 0x11,
  MAX_STREAMS_BIDI = 0x12,
  MAX_STREAMS_UNI = 0x13,
  DATA_BLOCKED = 0x14,
  STREAM_DATA_BLOCKED = 0x15,
  STREAMS_BLOCKED_BIDI = 0x16,
  STREAMS_BLOCKED_UNI = 0x17,
  CONNECTION_CLOSE = 0x1c,
  CONNECTION_CLOSE_APP = 0x1d,
  DATAGRAM_NO_LEN = 0x30,
  DATAGRAM = 0x31, // QUIC DATAGRAM with length
  // QMUX extensions
  QX_TRANSPORT_PARAMS = 0x3f5153300d0a0d0a,
  QX_PING = 0x15228c06, // placeholder, TBD in draft
  QX_PONG = 0x15228c07, // placeholder, TBD in draft
};

struct PaddingCapsule {
  size_t paddingLength{0};
};

struct WTResetStreamCapsule {
  uint64_t streamId{0};
  uint32_t appProtocolErrorCode{0};
  uint64_t reliableSize{0};
};

struct WTStopSendingCapsule {
  uint64_t streamId{0};
  uint32_t appProtocolErrorCode{0};
};

struct WTStreamCapsule {
  uint64_t streamId{0};
  std::unique_ptr<folly::IOBuf> streamData{};
  bool fin{};
};

struct WTMaxDataCapsule {
  uint64_t maximumData{0};
};

struct WTMaxStreamDataCapsule {
  uint64_t streamId{0};
  uint64_t maximumStreamData{0};
};

struct WTMaxStreamsCapsule {
  uint64_t maximumStreams{0};
};

struct WTDataBlockedCapsule {
  uint64_t maximumData{0};
};

struct WTStreamDataBlockedCapsule {
  uint64_t streamId{0};
  uint64_t maximumStreamData{0};
};

struct WTStreamsBlockedCapsule {
  uint64_t maximumStreams{0};
};

struct DatagramCapsule {
  std::unique_ptr<folly::IOBuf> httpDatagramPayload{};
};

struct CloseWebTransportSessionCapsule {
  uint32_t applicationErrorCode{0};
  std::string applicationErrorMessage{};
};

struct DrainWebTransportSessionCapsule {
  // no additional fields, length is 0
};

// Function declarations for parsing each capsule type
folly::Expected<PaddingCapsule, CapsuleCodec::ErrorCode> parsePadding(
    folly::io::Cursor& cursor, size_t length);

folly::Expected<WTResetStreamCapsule, CapsuleCodec::ErrorCode>
parseWTResetStream(folly::io::Cursor& cursor, size_t length);

folly::Expected<WTStopSendingCapsule, CapsuleCodec::ErrorCode>
parseWTStopSending(folly::io::Cursor& cursor, size_t length);

folly::Expected<WTStreamCapsule, CapsuleCodec::ErrorCode> parseWTStream(
    folly::io::Cursor& cursor, size_t length, bool fin);

folly::Expected<WTMaxDataCapsule, CapsuleCodec::ErrorCode> parseWTMaxData(
    folly::io::Cursor& cursor, size_t length);

folly::Expected<WTMaxStreamDataCapsule, CapsuleCodec::ErrorCode>
parseWTMaxStreamData(folly::io::Cursor& cursor, size_t length);

folly::Expected<WTMaxStreamsCapsule, CapsuleCodec::ErrorCode> parseWTMaxStreams(
    folly::io::Cursor& cursor, size_t length);

folly::Expected<WTDataBlockedCapsule, CapsuleCodec::ErrorCode>
parseWTDataBlocked(folly::io::Cursor& cursor, size_t length);

folly::Expected<WTStreamDataBlockedCapsule, CapsuleCodec::ErrorCode>
parseWTStreamDataBlocked(folly::io::Cursor& cursor, size_t length);

folly::Expected<WTStreamsBlockedCapsule, CapsuleCodec::ErrorCode>
parseWTStreamsBlocked(folly::io::Cursor& cursor, size_t length);

folly::Expected<DatagramCapsule, CapsuleCodec::ErrorCode> parseDatagram(
    folly::io::Cursor& cursor, size_t length);

folly::Expected<CloseWebTransportSessionCapsule, CapsuleCodec::ErrorCode>
parseCloseWebTransportSession(folly::io::Cursor& cursor, size_t length);

folly::Expected<DrainWebTransportSessionCapsule, CapsuleCodec::ErrorCode>
parseDrainWebTransportSession(size_t length);

//////// dispatchParsedFrame ////////

// Unifies the parse-check-dispatch pattern shared by CapsuleCodec and
// QmuxCodec:
//  - Returns early with the error if parsing fails
//  - Invokes the callback with the parsed frame if callback is non-null
//  - Returns success otherwise
template <typename ParseResult, typename Callback, typename MemFn>
folly::Expected<folly::Unit, CapsuleCodec::ErrorCode> dispatchParsedFrame(
    ParseResult&& result, Callback* cb, MemFn fn) {
  if (result.hasError()) {
    return folly::makeUnexpected(result.error());
  }
  if (cb) {
    (cb->*fn)(std::move(result.value()));
  }
  return folly::unit;
}

//////// Write helpers ////////

// change from std::optional<size_t> -> size_t (0 can signal error)?
using WriteResult = std::optional<size_t>;

// Function declarations for serializing each capsule type.
// Shared frame types accept a FrameProtocol parameter to select the wire
// format.  WT_CAPSULE (default) writes capsule TLV headers; QMUX writes the
// QUIC-style frame type with no outer length prefix.
WriteResult writePadding(folly::IOBufQueue& queue,
                         const PaddingCapsule& capsule);
WriteResult writeWTResetStream(
    folly::IOBufQueue& queue,
    const WTResetStreamCapsule& capsule,
    FrameProtocol protocol = FrameProtocol::WT_CAPSULE);
WriteResult writeWTStopSending(
    folly::IOBufQueue& queue,
    const WTStopSendingCapsule& capsule,
    FrameProtocol protocol = FrameProtocol::WT_CAPSULE);
WriteResult writeWTStream(folly::IOBufQueue& queue,
                          const WTStreamCapsule& capsule,
                          FrameProtocol protocol = FrameProtocol::WT_CAPSULE);
WriteResult writeWTMaxData(folly::IOBufQueue& queue,
                           const WTMaxDataCapsule& capsule,
                           FrameProtocol protocol = FrameProtocol::WT_CAPSULE);
WriteResult writeWTMaxStreamData(
    folly::IOBufQueue& queue,
    const WTMaxStreamDataCapsule& capsule,
    FrameProtocol protocol = FrameProtocol::WT_CAPSULE);
WriteResult writeWTMaxStreams(
    folly::IOBufQueue& queue,
    const WTMaxStreamsCapsule& capsule,
    bool isBidi,
    FrameProtocol protocol = FrameProtocol::WT_CAPSULE);
WriteResult writeWTDataBlocked(
    folly::IOBufQueue& queue,
    const WTDataBlockedCapsule& capsule,
    FrameProtocol protocol = FrameProtocol::WT_CAPSULE);
WriteResult writeWTStreamDataBlocked(
    folly::IOBufQueue& queue,
    const WTStreamDataBlockedCapsule& capsule,
    FrameProtocol protocol = FrameProtocol::WT_CAPSULE);
WriteResult writeWTStreamsBlocked(
    folly::IOBufQueue& queue,
    const WTStreamsBlockedCapsule& capsule,
    bool isBidi,
    FrameProtocol protocol = FrameProtocol::WT_CAPSULE);
WriteResult writeDatagram(folly::IOBufQueue& queue,
                          const DatagramCapsule& capsule,
                          FrameProtocol protocol = FrameProtocol::WT_CAPSULE);
// WT-only capsule types (no QMUX equivalent)
WriteResult writeCloseWebTransportSession(
    folly::IOBufQueue& queue, const CloseWebTransportSessionCapsule& capsule);
WriteResult writeDrainWebTransportSession(folly::IOBufQueue& queue);

} // namespace proxygen
