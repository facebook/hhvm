/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FifoReader.h"

#include <fcntl.h>

#include <algorithm>
#include <cstring>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <folly/io/async/EventBase.h>

namespace fs = boost::filesystem;

namespace facebook {
namespace memcache {

namespace {

constexpr size_t kHeaderMagicSize = sizeof(MessageHeader().magic());

uint8_t getVersion(const folly::IOBufQueue& bufQueue) {
  const size_t kLength = kHeaderMagicSize + sizeof(MessageHeader().version());
  CHECK(bufQueue.chainLength() >= kLength)
      << "Buffer queue length is smaller than (magic + version) bytes.";

  size_t offset = 0;
  auto buf = bufQueue.front();
  while ((offset + buf->length()) < kLength) {
    offset += buf->length();
    buf = buf->next();
  }
  return buf->data()[kLength - offset - 1];
}

PacketHeader parsePacketHeader(folly::IOBufQueue& bufQueue) {
  CHECK(bufQueue.chainLength() >= sizeof(PacketHeader))
      << "Invalid packet header buffer size!";

  auto buf = bufQueue.split(sizeof(PacketHeader));
  auto bytes = buf->coalesce();

  PacketHeader header;
  std::memcpy(&header, bytes.data(), sizeof(PacketHeader));

  CHECK(header.packetSize() <= kFifoMaxPacketSize)
      << "Packet too large: " << header.packetSize();

  return header;
}

MessageHeader parseMessageHeader(folly::IOBufQueue& bufQueue) {
  const size_t version = getVersion(bufQueue);
  const size_t messageHeaderSize = MessageHeader::size(version);

  CHECK(messageHeaderSize <= sizeof(MessageHeader))
      << "MessageHeader struct cannot hold message header data";
  CHECK(bufQueue.chainLength() >= messageHeaderSize)
      << "Invalid message header buffer size!";

  auto buf = bufQueue.split(messageHeaderSize);

  MessageHeader header;
  std::memcpy(&header, buf->coalesce().data(), messageHeaderSize);

  return header;
}

bool isMessageHeader(const folly::IOBufQueue& bufQueue) {
  CHECK(bufQueue.chainLength() >= kHeaderMagicSize)
      << "Buffer queue length is smaller than magic bytes.";

  uint32_t magic = 0;
  size_t i = 0;
  auto buf = bufQueue.front();
  while (i < sizeof(uint32_t)) {
    size_t j = 0;
    while (j < buf->length() && i < sizeof(uint32_t)) {
      // data is sent in little endian format.
      magic += (buf->data()[j] << (i * CHAR_BIT));
      ++i;
      ++j;
    }
    buf = buf->next();
  }
  magic = folly::Endian::little(magic);

  return magic == MessageHeader().magic();
}

} // anonymous namespace

FifoReadCallback::FifoReadCallback(
    std::string fifoName,
    const MessageReadyFn& messageReady) noexcept
    : fifoName_(std::move(fifoName)), messageReady_(messageReady) {}

void FifoReadCallback::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  auto res = readBuffer_.preallocate(kMinSize, PIPE_BUF);
  *bufReturn = res.first;
  *lenReturn = res.second;
}

void FifoReadCallback::readDataAvailable(size_t len) noexcept {
  try {
    readBuffer_.postallocate(len);

    // Process any pending packet headers.
    if (pendingHeader_) {
      if (readBuffer_.chainLength() < pendingHeader_->packetSize()) {
        return;
      }
      forwardMessage(
          pendingHeader_.value(),
          readBuffer_.split(pendingHeader_->packetSize()));
      pendingHeader_.reset();
    }

    while (readBuffer_.chainLength() >= kHeaderMagicSize) {
      if (isMessageHeader(readBuffer_)) {
        if (readBuffer_.chainLength() < sizeof(MessageHeader)) {
          // Wait for more data
          return;
        }
        handleMessageHeader(parseMessageHeader(readBuffer_));
      }

      if (readBuffer_.chainLength() < sizeof(PacketHeader)) {
        // Wait for more data
        return;
      }
      auto packetHeader = parsePacketHeader(readBuffer_);
      if (packetHeader.packetSize() > readBuffer_.chainLength()) {
        // Wait for more data.
        pendingHeader_.assign(std::move(packetHeader));
        return;
      }

      forwardMessage(
          packetHeader, readBuffer_.split(packetHeader.packetSize()));
    }
  } catch (const std::exception& ex) {
    CHECK(false) << "Unexpected exception: " << ex.what();
  }
}

void FifoReadCallback::handleMessageHeader(MessageHeader msgHeader) noexcept {
  from_ = msgHeader.getLocalAddress();
  to_ = msgHeader.getPeerAddress();
  typeId_ = msgHeader.typeId();
  msgStartTime_ = msgHeader.timeUs();
  if (msgHeader.direction() == MessageDirection::Received) {
    std::swap(from_, to_);
  }
  if (!msgHeader.routerName()[0]) {
    carbonRouterName_ = facebook::memcache::MemcacheRouterInfo::name;
  } else {
    carbonRouterName_ = msgHeader.routerName();
  }
}

void FifoReadCallback::forwardMessage(
    const PacketHeader& header,
    std::unique_ptr<folly::IOBuf> buf) {
  auto data = buf->coalesce();
  CHECK(data.size() == header.packetSize()) << "Invalid header buffer size!";

  if (typeId_ != 0) {
    messageReady_(
        header.connectionId(),
        header.packetId(),
        std::move(from_),
        std::move(to_),
        typeId_,
        msgStartTime_,
        carbonRouterName_,
        data);
    typeId_ = 0;
  } else {
    VLOG(2) << "Type id is 0. Skipping message.";
  }
}

void FifoReadCallback::readEOF() noexcept {
  LOG(INFO) << "Fifo \"" << fifoName_ << "\" disconnected";
}

void FifoReadCallback::readErr(const folly::AsyncSocketException& e) noexcept {
  LOG(ERROR) << "Error reading fifo \"" << fifoName_ << "\": " << e.what();
}

FifoReaderManager::FifoReaderManager(
    folly::EventBase& evb,
    MessageReadyFn messageReady,
    std::string dir,
    std::unique_ptr<boost::regex> filenamePattern)
    : evb_(evb),
      messageReady_(std::move(messageReady)),
      directory_(std::move(dir)),
      filenamePattern_(std::move(filenamePattern)) {
  runScanDirectory();
}

std::vector<std::string> FifoReaderManager::getMatchedFiles() const {
  std::vector<std::string> fifos;

  try {
    if (!fs::exists(directory_)) {
      LOG(ERROR) << "Directory \"" << directory_ << "\" not found.";
    } else if (!fs::is_directory(directory_)) {
      LOG(ERROR) << "Path \"" << directory_ << "\" is not a directory.";
    } else {
      fs::directory_iterator endIt; // default construction = end iterator.
      for (fs::directory_iterator it(directory_); it != endIt; ++it) {
        if (fs::is_directory(it->status())) {
          continue;
        }
        auto& path = it->path();
        if (!filenamePattern_ ||
            boost::regex_search(
                path.filename().string(),
                *filenamePattern_,
                boost::regex_constants::match_default)) {
          fifos.emplace_back(path.string());
        }
      }
    }
  } catch (const fs::filesystem_error& ex) {
    LOG(ERROR) << "Failed to find fifos: " << ex.what();
  }

  return fifos;
}

void FifoReaderManager::runScanDirectory() {
  auto fifos = getMatchedFiles();
  for (const auto& fifo : fifos) {
    if (fifoReaders_.find(fifo) != fifoReaders_.end()) {
      continue;
    }
    auto fd = ::open(fifo.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
      auto pipeReader = folly::AsyncPipeReader::UniquePtr(
          new folly::AsyncPipeReader(&evb_, folly::NetworkSocket::fromFd(fd)));
      auto callback = std::make_unique<FifoReadCallback>(fifo, messageReady_);
      pipeReader->setReadCB(callback.get());
      fifoReaders_.emplace(
          fifo, FifoReader(std::move(pipeReader), std::move(callback)));
    } else {
      PLOG(WARNING) << "Error opening fifo: " << fifo;
    }
  }

  evb_.runAfterDelay(
      [this]() { runScanDirectory(); }, kPollDirectoryIntervalMs);
}

void FifoReaderManager::unregisterCallbacks() {
  for (auto& fifoReader : fifoReaders_) {
    fifoReader.second.first->setReadCB(nullptr);
  }
}

} // namespace memcache
} // namespace facebook
