/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/regex.hpp>

#include <folly/Optional.h>
#include <folly/SocketAddress.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncPipe.h>
#include <folly/io/async/AsyncSocketException.h>

#include "mcrouter/lib/debug/ConnectionFifoProtocol.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"

namespace folly {
class EventBase;
} // namespace folly

namespace facebook {
namespace memcache {

class FifoReader;

/**
 * Function called when a message packet is completely read from the fifo.
 *
 * @param connectionId  Id of the connection.
 * @param packetId      Id of the packet.
 * @param from          Address of the endpoint that sent the message.
 * @param to            Address of the endpoint that received the message.
 * @param typeId        Id of the type of the request/reply.
 * @param msgStartTime  The time the request/reply was received/send.
 * @param routerName    Generated router name that helps identify protocol.
 * @param data          The data of the message.
 */
using MessageReadyFn = std::function<void(
    uint64_t connectionId,
    uint64_t packetId,
    folly::SocketAddress from,
    folly::SocketAddress to,
    uint32_t typeId,
    uint64_t msgStartTime,
    std::string routerName,
    folly::ByteRange data)>;

class FifoReadCallback : public folly::AsyncReader::ReadCallback {
 public:
  FifoReadCallback(
      std::string fifoName,
      const MessageReadyFn& messageReady) noexcept;

  void getReadBuffer(void** bufReturn, size_t* lenReturn) final;
  void readDataAvailable(size_t len) noexcept final;
  void readEOF() noexcept final;
  void readErr(const folly::AsyncSocketException& ex) noexcept final;

 private:
  static constexpr uint64_t kMinSize{256};
  folly::IOBufQueue readBuffer_{folly::IOBufQueue::cacheChainLength()};
  const std::string fifoName_;
  const MessageReadyFn& messageReady_;

  // Indicates if there is a pending message, i.e. a header has being read
  // (pendingHeader_) but its data hasn't being processed yet
  folly::Optional<PacketHeader> pendingHeader_;

  // Addresses of the endpoints of the message currently being read.
  folly::SocketAddress from_;
  folly::SocketAddress to_;

  uint32_t typeId_{0};
  uint64_t msgStartTime_{0};

  // Name of the carbon router.
  std::string carbonRouterName_;

  void forwardMessage(
      const PacketHeader& header,
      std::unique_ptr<folly::IOBuf> buf);

  void handleMessageHeader(MessageHeader msgHeader) noexcept;
};

/**
 * Manages all fifo readers in a directory.
 */
class FifoReaderManager {
 public:
  /**
   * Builds FifoReaderManager and starts watching "dir" for fifos
   * that match "filenamePattern".
   * If a fifo with a name that matches "filenamePattern" is found, a
   * folly:AsyncPipeReader for it is created and scheduled in "evb".
   *
   * @param evb             EventBase to run FifoReaderManager and
   *                        its FifoReaders.
   * @param messageReadyCb  Callback to be called when a message is completely
   *                        read from the fifo.
   * @param dir             Directory to watch.
   * @param filenamePattern Regex that file names must match.
   */
  FifoReaderManager(
      folly::EventBase& evb,
      MessageReadyFn messageReady,
      std::string dir,
      std::unique_ptr<boost::regex> filenamePattern);

  // non-copyable
  FifoReaderManager(const FifoReaderManager&) = delete;
  FifoReaderManager& operator=(const FifoReaderManager&) = delete;

  /**
   * Unregisters all fifo readers
   */
  void unregisterCallbacks();

 private:
  using FifoReader = std::pair<
      folly::AsyncPipeReader::UniquePtr,
      std::unique_ptr<FifoReadCallback>>;

  static constexpr size_t kPollDirectoryIntervalMs = 1000;
  folly::EventBase& evb_;
  MessageReadyFn messageReady_;
  const std::string directory_;
  const std::unique_ptr<boost::regex> filenamePattern_;
  std::unordered_map<std::string, FifoReader> fifoReaders_;

  std::vector<std::string> getMatchedFiles() const;
  void runScanDirectory();
};
} // namespace memcache
} // namespace facebook
