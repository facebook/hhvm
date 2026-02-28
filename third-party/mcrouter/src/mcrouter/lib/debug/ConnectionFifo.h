/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/io/async/AsyncTransport.h>

#include "mcrouter/lib/debug/ConnectionFifoProtocol.h"
#include "mcrouter/lib/debug/Fifo.h"

namespace facebook {
namespace memcache {

/**
 * Responsible for dumping traffic of a single connection to FIFO.
 *
 * Notes:
 *  - At any given time, there can be at most one message being written
 *    to the FIFO.
 */
class ConnectionFifo {
 public:
  /**
   * Builds a dumb ConnectionFifo, that never connects.
   */
  ConnectionFifo() noexcept;

  /**
   * Builds ConnectionFifo
   *
   * @param debugFifo   Underlying FIFO to which the data will be written.
   * @param transport   Transport from which data will be mirrored.
   * @param routerName  Name of the router.
   */
  ConnectionFifo(
      std::shared_ptr<Fifo> debugFifo,
      const folly::AsyncTransportWrapper* transport,
      folly::StringPiece routerName) noexcept;

  /**
   * Tells whether or not there is a client connected to the underlying FIFO.
   */
  bool isConnected() const noexcept;

  /**
   * Starts a new message.
   *
   * @param direction   Whether the data was received or sent by connection.
   * @param typeId      Id of the type of the message.
   */
  bool startMessage(MessageDirection direction, uint32_t typeId) noexcept;

  /**
   * Writes data to the FIFO, but only if there is reader (i.e. mcpiper)
   * connected to it.
   *
   * Before writting data to the fifo, a new message must be started
   * (i.e. startMessage() should be called).
   */
  bool writeData(const struct iovec* iov, size_t iovcnt) noexcept;
  bool writeData(const void* buf, size_t len) noexcept;

 private:
  std::shared_ptr<Fifo> debugFifo_;
  MessageHeader currentMessageHeader_;
  uint32_t nextPacketId_{0};
};

} // namespace memcache
} // namespace facebook
