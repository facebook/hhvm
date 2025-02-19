/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <quic/api/QuicSocket.h>
#include <wangle/acceptor/TransportInfo.h>

namespace proxygen {

/**
 * Session-level protocol info.
 */
struct QuicProtocolInfo : public wangle::ProtocolInfo {
  ~QuicProtocolInfo() override = default;

  folly::Optional<quic::ConnectionId> clientChosenDestConnectionId;
  folly::Optional<quic::ConnectionId> clientConnectionId;
  folly::Optional<quic::ConnectionId> serverConnectionId;
  folly::Optional<quic::TransportSettings> transportSettings;
  folly::Optional<std::string> fingerprint;
  folly::Optional<quic::QuicVersion> quicVersion;

  uint32_t ptoCount{0};
  uint32_t totalPTOCount{0};
  uint64_t totalTransportBytesSent{0};
  uint64_t totalTransportBytesRecvd{0};
  bool usedZeroRtt{false};
};

inline void initQuicProtocolInfo(QuicProtocolInfo& quicInfo,
                                 const quic::QuicSocket& sock) {
  quicInfo.clientChosenDestConnectionId =
      sock.getClientChosenDestConnectionId();
  quicInfo.clientConnectionId = sock.getClientConnectionId();
  quicInfo.serverConnectionId = sock.getServerConnectionId();
  quicInfo.quicVersion =
      sock.getState() ? sock.getState()->version : folly::none;
}

inline void updateQuicProtocolInfo(QuicProtocolInfo& quicInfo,
                                   const quic::QuicSocket& sock) {
  auto curQuicInfo = sock.getTransportInfo();
  quicInfo.ptoCount = curQuicInfo.ptoCount;
  quicInfo.totalPTOCount = curQuicInfo.totalPTOCount;
  quicInfo.totalTransportBytesSent = curQuicInfo.bytesSent;
  quicInfo.totalTransportBytesRecvd = curQuicInfo.bytesRecvd;
  quicInfo.transportSettings = sock.getTransportSettings();
  quicInfo.usedZeroRtt = curQuicInfo.usedZeroRtt;
}

/**
 *  Stream level protocol info. Contains all data from
 *  the sessinon info, plus stream-specific information.
 *  This structure is owned by each individual stream,
 *  and is updated when requested.
 *  If instance of HQ Transport Stream outlives the corresponding QUIC socket,
 *  has been destroyed, this structure will contain the last snapshot
 *  of the data received from the QUIC socket.
 *
 * Usage:
 *   TransportInfo tinfo;
 *   txn.getCurrentTransportInfo(&tinfo); // txn is the HTTP transaction object
 *   auto streamInfo = dynamic_cast<QuicStreamProtocolInfo>(tinfo.protocolInfo);
 *   if (streamInfo) {
 *      // stream level AND connection level info is available
 *   };
 *   auto connectionInfo = dynamic_cast<QuicProtocolInfo>(tinfo.protocolInfo);
 *   if (connectionInfo) {
 *     // ONLY connection level info is available. No stream level info.
 *   }
 *
 */
struct QuicStreamProtocolInfo : public QuicProtocolInfo {

  // Slicing assignment operator to initialize the per-stream protocol info
  // with the values of the per-session protocol info.
  QuicStreamProtocolInfo& operator=(const QuicProtocolInfo& other) {
    if (this != &other) {
      *(static_cast<QuicProtocolInfo*>(this)) = other;
    }
    return *this;
  }

  quic::QuicSocket::StreamTransportInfo streamTransportInfo;
  // NOTE: when the control stream latency stats will be reintroduced,
  // collect it here.
};

} // namespace proxygen
