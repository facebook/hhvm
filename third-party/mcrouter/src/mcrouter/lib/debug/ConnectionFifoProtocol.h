/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <limits.h>

#include <folly/Portability.h>
#include <folly/Range.h>
#include <folly/SocketAddress.h>
#include <folly/lang/Bits.h>

namespace facebook {
namespace memcache {

/**
 * Represents the direction of a ConnectionFifo message.
 */
enum class MessageDirection : uint8_t { Sent = 0, Received = 1 };

constexpr folly::StringPiece kUnixSocketPrefix{"US:"};

/**
 * Header of the message of ConnectionFifo.
 */
struct FOLLY_PACK_ATTR MessageHeader {
 public:
  constexpr static size_t kAddressMaxSize = 40;
  constexpr static size_t kRouterNameMaxSize = 30;

  uint32_t magic() const {
    return folly::Endian::little(magic_);
  }
  uint8_t version() const {
    return version_;
  }
  const char* peerAddress() const {
    return peerAddress_;
  }
  uint16_t peerPort() const {
    return folly::Endian::little(peerPort_);
  }
  uint64_t connectionId() const {
    return folly::Endian::little(connectionId_);
  }
  uint16_t localPort() const {
    return folly::Endian::little(localPort_);
  }
  MessageDirection direction() const {
    return direction_;
  }
  uint32_t typeId() const {
    return folly::Endian::little(typeId_);
  }
  uint64_t timeUs() const {
    return folly::Endian::little(timeUs_);
  }
  const char* routerName() const {
    return routerName_;
  }

  char* peerAddressModifiable() {
    return peerAddress_;
  }
  void setPeerPort(uint16_t val) {
    peerPort_ = folly::Endian::little(val);
  }
  void setConnectionId(uint64_t val) {
    connectionId_ = folly::Endian::little(val);
  }
  void setLocalPort(uint16_t val) {
    localPort_ = folly::Endian::little(val);
  }
  void setDirection(MessageDirection val) {
    direction_ = val;
  }
  void setTypeId(uint32_t val) {
    typeId_ = folly::Endian::little(val);
  }
  void setTimeUs(uint64_t val) {
    timeUs_ = folly::Endian::little(val);
  }
  char* routerNameModifiable() {
    return routerName_;
  }

  folly::SocketAddress getLocalAddress();
  folly::SocketAddress getPeerAddress();

  bool isUnixDomainSocket() const;

  static size_t size(uint8_t v);

 private:
  // Control fields
  uint32_t magic_ = folly::Endian::little<uint32_t>(0xfaceb00c);
  uint8_t version_{4};

  // Peer address fields
  char peerAddress_[kAddressMaxSize]{'\0'}; // 0-terminated string of address
  uint16_t peerPort_{0};

  // Message fields
  uint64_t connectionId_{0};

  // Local address fields
  uint16_t localPort_{0};

  // Direction of the message sent
  MessageDirection direction_{MessageDirection::Sent};

  // Id of the type
  uint32_t typeId_{0};

  // Number of micro-seconds elapsed sience epoch.
  uint64_t timeUs_{0};

  char routerName_[kRouterNameMaxSize]{'\0'}; // 0-terminated string of address
};

/**
 * Header of the packet.
 * FIFO's can only write up to PIPE_BUF (tipically 4096 in linux) bytes
 * atomically at a time. For that reason, calls to Fifo::writeIfConnected()
 * are broke down into packets.
 */
struct FOLLY_PACK_ATTR PacketHeader {
 public:
  uint64_t connectionId() const {
    return folly::Endian::little(connectionId_);
  }
  uint32_t packetSize() const {
    return folly::Endian::little(packetSize_);
  }
  uint32_t packetId() const {
    return folly::Endian::little(packetId_);
  }
  void setConnectionId(uint64_t val) {
    connectionId_ = folly::Endian::little(val);
  }
  void setPacketSize(uint32_t val) {
    packetSize_ = folly::Endian::little(val);
  }
  void setPacketId(uint32_t val) {
    packetId_ = folly::Endian::little(val);
  }

 private:
  uint64_t connectionId_{0};
  uint32_t packetSize_{0};
  uint32_t packetId_{0};
};
constexpr uint32_t kFifoMaxPacketSize = PIPE_BUF - sizeof(PacketHeader);
static_assert(
    PIPE_BUF > sizeof(MessageHeader) + sizeof(PacketHeader),
    "sizeof(PacketHeader) + sizeof(MessageHeader) "
    "must be smaller than PIPE_BUF.");

} // namespace memcache
} // namespace facebook
