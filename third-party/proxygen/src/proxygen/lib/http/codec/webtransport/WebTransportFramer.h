/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/CapsuleCodec.h>
#include <quic/codec/QuicInteger.h>

namespace proxygen {

//////// Types ////////

enum class CapsuleType : uint32_t {
  PADDING = 0x190B4D38,
  WT_RESET_STREAM = 0x190B4D39,
  WT_STOP_SENDING = 0x190B4D3A,
  WT_STREAM = 0x190B4D3B,
  WT_STREAM_WITH_FIN = 0x190B4D3C,
  WT_MAX_DATA = 0x190B4D3D,
  WT_MAX_STREAM_DATA = 0x190B4D3E,
  WT_MAX_STREAMS = 0x190B4D3F,
  WT_DATA_BLOCKED = 0x190B4D41,
  WT_STREAM_DATA_BLOCKED = 0x190B4D42,
  WT_STREAMS_BLOCKED = 0x190B4D43,
  DATAGRAM = 0x00,
  CLOSE_WEBTRANSPORT_SESSION = 0x190B4D45,
  DRAIN_WEBTRANSPORT_SESSION = 0x190B4D46
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

using WriteResult = folly::Expected<size_t, quic::TransportErrorCode>;

// Function declarations for serializing each capsule type
WriteResult writePadding(folly::IOBufQueue& queue,
                         const PaddingCapsule& capsule);
WriteResult writeWTResetStream(folly::IOBufQueue& queue,
                               const WTResetStreamCapsule& capsule);
WriteResult writeWTStopSending(folly::IOBufQueue& queue,
                               const WTStopSendingCapsule& capsule);
WriteResult writeWTStream(folly::IOBufQueue& queue,
                          const WTStreamCapsule& capsule);
WriteResult writeWTMaxData(folly::IOBufQueue& queue,
                           const WTMaxDataCapsule& capsule);
WriteResult writeWTMaxStreamData(folly::IOBufQueue& queue,
                                 const WTMaxStreamDataCapsule& capsule);
WriteResult writeWTMaxStreams(folly::IOBufQueue& queue,
                              const WTMaxStreamsCapsule& capsule);
WriteResult writeWTDataBlocked(folly::IOBufQueue& queue,
                               const WTDataBlockedCapsule& capsule);
WriteResult writeWTStreamDataBlocked(folly::IOBufQueue& queue,
                                     const WTStreamDataBlockedCapsule& capsule);
WriteResult writeWTStreamsBlocked(folly::IOBufQueue& queue,
                                  const WTStreamsBlockedCapsule& capsule);
WriteResult writeDatagram(folly::IOBufQueue& queue,
                          const DatagramCapsule& capsule);
WriteResult writeCloseWebTransportSession(
    folly::IOBufQueue& queue, const CloseWebTransportSessionCapsule& capsule);
WriteResult writeDrainWebTransportSession(folly::IOBufQueue& queue);

} // namespace proxygen
