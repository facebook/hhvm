/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ConnectionFifo.h"

#include <chrono>

#include <folly/Range.h>

namespace facebook {
namespace memcache {

namespace {

uint64_t timeSinceEpoch() {
  using namespace std::chrono;
  return duration_cast<microseconds>(steady_clock::now().time_since_epoch())
      .count();
}

class PipeIov {
 public:
  const iovec* iov() const {
    return iov_;
  }
  size_t size() const {
    return index_;
  }
  bool full() const {
    return index_ >= kMaxLen || remSize_ == 0;
  }

  void reset() {
    index_ = 0;
    remSize_ = PIPE_BUF;
  }
  size_t append(void* buf, size_t len) {
    assert(!full());

    size_t numBytes = std::min(len, remSize_);
    iov_[index_].iov_base = buf;
    iov_[index_].iov_len = numBytes;
    remSize_ -= numBytes;
    ++index_;
    return numBytes;
  }

 private:
  static constexpr size_t kMaxLen = 16;
  iovec iov_[kMaxLen];
  size_t index_ = 0;
  size_t remSize_ = PIPE_BUF;

  static_assert(
      IOV_MAX >= kMaxLen,
      "IOV_MAX must be larger than PipeIov::kMaxLen.");
};

/**
 * Helper to keep state of iteration over const iovec.
 */
class IovecIterator {
 public:
  IovecIterator(const struct iovec* iov, size_t iovcnt)
      : iov_(iov), iovcnt_(iovcnt) {
    assert(iovcnt > 0);

    curBuf_ = reinterpret_cast<char*>(iov_[0].iov_base);
    curBufLen_ = iov_[0].iov_len;
  }

  char* currentBuffer() const {
    return curBuf_;
  }
  size_t currentBufferLength() const {
    return curBufLen_;
  }
  bool hasData() const {
    return index_ < iovcnt_;
  }

  void advance(size_t bytes) {
    assert(hasData());
    assert(bytes <= curBufLen_);

    if (bytes < curBufLen_) {
      curBuf_ += bytes;
      curBufLen_ -= bytes;
    } else { // (bytes == bufLen)
      ++index_;
      if (hasData()) {
        curBuf_ = reinterpret_cast<char*>(iov_[index_].iov_base);
        curBufLen_ = iov_[index_].iov_len;
      }
    }
  }

 private:
  const struct iovec* iov_;
  const size_t iovcnt_;

  size_t index_{0};
  char* curBuf_{nullptr};
  size_t curBufLen_{0};
};

MessageHeader buildMsgHeader(
    const folly::AsyncTransportWrapper* transport,
    folly::StringPiece routerName) {
  MessageHeader header;
  header.setConnectionId(reinterpret_cast<uintptr_t>(transport));

  if (!transport) {
    return header;
  }

  try {
    folly::SocketAddress address;

    transport->getPeerAddress(&address);
    if (address.getFamily() == AF_INET || address.getFamily() == AF_INET6) {
      address.getAddressStr(
          header.peerAddressModifiable(), MessageHeader::kAddressMaxSize);
      header.setPeerPort(address.getPort());
      header.setLocalPort(address.getPort());
    } else if (address.getFamily() == AF_UNIX) {
      // For unix sockets, just localAddress has the path.
      transport->getLocalAddress(&address);
      std::snprintf(
          header.peerAddressModifiable(),
          MessageHeader::kAddressMaxSize,
          "%s%s",
          kUnixSocketPrefix.data(),
          address.getPath().c_str());
    }

    transport->getLocalAddress(&address);
  } catch (const std::exception& e) {
    VLOG(2) << "Error getting host/port to write to debug fifo: " << e.what();
  }

  // set router name
  header.routerNameModifiable()[0] = '\0';
  if (!routerName.empty()) {
    int res = std::snprintf(
        header.routerNameModifiable(),
        MessageHeader::kRouterNameMaxSize,
        "%s",
        routerName.str().c_str());
    if (res < 0) {
      LOG(ERROR) << "Error writing router name '" << routerName
                 << "' to debug fifo";
    }
  }

  return header;
}

} // anonymous namespace

ConnectionFifo::ConnectionFifo() noexcept {}

ConnectionFifo::ConnectionFifo(
    std::shared_ptr<Fifo> debugFifo,
    const folly::AsyncTransportWrapper* transport,
    folly::StringPiece routerName) noexcept
    : debugFifo_(std::move(debugFifo)),
      currentMessageHeader_(buildMsgHeader(transport, routerName)) {}

bool ConnectionFifo::isConnected() const noexcept {
  return debugFifo_ && debugFifo_->isConnected();
}

bool ConnectionFifo::startMessage(
    MessageDirection direction,
    uint32_t typeId) noexcept {
  if (!isConnected()) {
    return false;
  }
  currentMessageHeader_.setDirection(direction);
  currentMessageHeader_.setTypeId(typeId);
  currentMessageHeader_.setTimeUs(timeSinceEpoch());
  nextPacketId_ = 0;
  return true;
}

bool ConnectionFifo::writeData(const void* buf, size_t len) noexcept {
  if (!isConnected()) {
    return false;
  }
  iovec iov[1];
  iov[0].iov_base = const_cast<void*>(buf);
  iov[0].iov_len = len;
  return writeData(iov, 1);
}
bool ConnectionFifo::writeData(
    const struct iovec* iov,
    size_t iovcnt) noexcept {
  // This method breaks data into packets and write them to the FIFO in a
  // specific format, described bellow:
  //  - First, the MessageHeader is written, with the following data formatted
  //    in little endian. The order is exactly as stated bellow:
  //      - magic           - Magic bytes to identify that this is a message
  //                          header. Value: 0xfaceb00c
  //      - version         - Version of the protocol.
  //      - peer ipAddress  - char[40] containing a 0-terminated string
  //                          representation of the peer's ip address.
  //      - peer port       - Peer port used for communication.
  //      - connection id   - Id of the connection that this message belongs to.
  //      - local port      - Local port used for communication.
  //      - direction       - Direction of the message (sent or received).
  //      - type id         - Id of the type of the request. For requests,
  //                          it's always odd, for responses it's reqTypeId + 1.
  //      - time            - Time in micros since epoch of when the message
  //                          originated.
  //  - After the message header, the data of the message is divided in
  //    packets of at most PIPE_BUF bytes.
  //  - Each packet is composed of two parts: header and body.
  //    - The packet header contains the following metadata about the packet:
  //      - connection id   - the id of the connection the packet belogs to.
  //                          8 bytes, little endian.
  //      - packet id       - Incremental id of the packet, starting at 0.
  //                          2 bytes, little endian.
  //      - packet size     - The size of the body of the packet.
  //                          2 bytes, little endian.
  //    - The body is the data itself (i.e. contents of iov).
  //  - Each packet is atomically written to the pipe. That means that
  //    the entire packet (header and body) will appear together in the
  //    pipe, as expected. The various packets of a message might be
  //    interleaved with packets from another message.
  //  - The entire message is NOT guaranteed to be written. Even though each
  //    packet is either completely written or not present at all, the message
  //    transmission might be interrupted - meaning that the last packets might
  //    be missing.
  //
  // Here is the layout of a message:
  // ---------------------------------------------------------
  // | MESSAGE HEADER | PACKET 1 | PACKET 2 | ... | PACKET N |
  // ---------------------------------------------------------
  // Where each packet has the following format:
  // -------------------------------
  // | PACKET HEADER | PACKET BODY |
  // -------------------------------

  if (!isConnected() || iovcnt == 0) {
    return false;
  }

  PipeIov pipeIov;
  IovecIterator iovIter(iov, iovcnt);

  // Write header only if this is a brand new message.
  if (nextPacketId_ == 0) {
    pipeIov.append(&currentMessageHeader_, sizeof(MessageHeader));
  }

  // Create packet header
  PacketHeader pkgHeader;
  pkgHeader.setConnectionId(currentMessageHeader_.connectionId());

  // Add packet header
  pipeIov.append(&pkgHeader, sizeof(PacketHeader));

  // Send data
  uint32_t packetSize = 0;
  while (iovIter.hasData()) {
    // Build pipeIov
    while (iovIter.hasData() && !pipeIov.full()) {
      auto bytesAppended = pipeIov.append(
          iovIter.currentBuffer(), iovIter.currentBufferLength());
      iovIter.advance(bytesAppended);
      packetSize += bytesAppended;
    }
    pkgHeader.setPacketSize(packetSize);
    pkgHeader.setPacketId(nextPacketId_);

    // Write to pipe
    if (!debugFifo_->write(pipeIov.iov(), pipeIov.size())) {
      return false;
    }

    ++nextPacketId_;
    packetSize = 0;
    pipeIov.reset();
    pipeIov.append(&pkgHeader, sizeof(PacketHeader));
  }

  return true;
}

} // namespace memcache
} // namespace facebook
