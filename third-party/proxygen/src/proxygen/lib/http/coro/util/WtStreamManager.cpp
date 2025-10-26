/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/coro/util/WtStreamManager.h>

#include <folly/logging/xlog.h>

namespace {

constexpr uint8_t kStreamIdInc = 0x04;

}

namespace proxygen::coro::detail {

/*static*/ auto WtStreamManager::selfNextStreams(WtDir dir, WtMaxStreams max)
    -> NextStreams {
  // default client-initiated streams for dir == client; adjusted if dir is
  // server
  NextStreams streams{.bidi = 0x00, .uni = 0x02, .max = max};
  if (dir == WtDir::Server) {
    streams.bidi += 0x01;
    streams.uni += 0x01;
  }
  return streams;
}

/*static*/ auto WtStreamManager::peerNextStreams(WtDir dir, WtMaxStreams max)
    -> NextStreams {
  // just invert direction and invoke selfNextStreams
  dir = static_cast<WtDir>(dir ^ 0x01u);
  auto streams = selfNextStreams(dir, max);
  return streams;
}

WtStreamManager::WtStreamManager(WtDir dir,
                                 WtMaxStreams self,
                                 WtMaxStreams peer)
    : dir_(dir),
      self_(selfNextStreams(dir, self)),
      peer_(peerNextStreams(dir, peer)) {
  XCHECK(dir <= WtDir::Server) << "invalid dir";
}

WtStreamManager::~WtStreamManager() = default;

bool WtStreamManager::isSelf(uint64_t streamId) const {
  return (streamId & 0x01) == uint8_t(dir_);
}
bool WtStreamManager::isPeer(uint64_t streamId) const {
  return !isSelf(streamId);
}

bool WtStreamManager::isUni(uint64_t streamId) const {
  return (streamId & 0x02) > 0;
}
bool WtStreamManager::isBidi(uint64_t streamId) const {
  return !isUni(streamId);
}

bool WtStreamManager::isIngress(uint64_t streamId) const {
  return isBidi(streamId) || (isPeer(streamId) && isUni(streamId));
}
bool WtStreamManager::isEgress(uint64_t streamId) const {
  return isBidi(streamId) || (isSelf(streamId) && isUni(streamId));
}

uint64_t* WtStreamManager::nextExpectedStream(uint64_t streamId) {
  auto& next = isSelf(streamId) ? self_ : peer_;
  auto& id = (isBidi(streamId) ? next.bidi : next.uni);
  return (id == streamId) ? &id : nullptr;
}

struct ReadHandle : public WebTransport::StreamReadHandle {
  // why doesn't using StreamReadHandle::StreamReadHandle work here?
  ReadHandle(uint64_t id) : StreamReadHandle(id) {
  }
  using StreamData = WebTransport::StreamData;
  using ErrCode = WebTransport::ErrorCode;
  // StreamReadHandle overrides
  folly::SemiFuture<StreamData> readStreamData() override;
  folly::Expected<folly::Unit, ErrCode> stopSending(uint32_t error) override;
  void enqueue(StreamData&& data);
};
struct WriteHandle : public WebTransport::StreamWriteHandle {
  // why doesn't using StreamWriteHandle::StreamWriteHandle work here?
  WriteHandle(uint64_t id) : StreamWriteHandle(id) {
  }
  using FcState = WebTransport::FCState;
  using ErrCode = WebTransport::ErrorCode;
  // StreamWriteHandle overrides
  folly::Expected<FcState, ErrCode> writeStreamData(
      std::unique_ptr<folly::IOBuf> data,
      bool fin,
      WebTransport::ByteEventCallback* byteEventCallback) override;
  folly::Expected<folly::Unit, ErrCode> resetStream(uint32_t error) override;
  folly::Expected<folly::Unit, ErrCode> setPriority(uint8_t level,
                                                    uint32_t order,
                                                    bool incremental) override;
  folly::Expected<folly::SemiFuture<uint64_t>, ErrCode> awaitWritable()
      override;

  WebTransport::StreamData dequeue();
};
struct WtStreamManager::BidiHandle {
  WriteHandle wh;
  ReadHandle rh;

  BidiHandle(uint64_t id) : wh(id), rh(id) {
  }

  static std::unique_ptr<BidiHandle> make(uint64_t id) {
    return std::make_unique<BidiHandle>(id);
  }
};

WebTransport::StreamWriteHandle* WtStreamManager::getEgressHandle(
    uint64_t streamId) {
  if (!isEgress(streamId)) {
    return nullptr; // egress handle invalid
  }

  // create if next expected stream
  if (auto* next = nextExpectedStream(streamId)) {
    streams_.emplace(streamId, BidiHandle::make(streamId));
    *next += kStreamIdInc;
  }

  auto it = streams_.find(streamId);
  return it != streams_.end() ? &it->second->wh : nullptr;
}

WebTransport::StreamReadHandle* WtStreamManager::getIngressHandle(
    uint64_t streamId) {
  if (!isIngress(streamId)) {
    return nullptr; // ingress handle invalid
  }
  // create if next expected stream
  if (auto* next = nextExpectedStream(streamId)) {
    streams_.emplace(streamId, BidiHandle::make(streamId));
    *next += kStreamIdInc;
  }

  auto it = streams_.find(streamId);
  return it != streams_.end() ? &it->second->rh : nullptr;
}

WebTransport::BidiStreamHandle WtStreamManager::getBidiHandle(
    uint64_t streamId) {
  return {getIngressHandle(streamId), getEgressHandle(streamId)};
}

WtStreamManager::WtWh* WtStreamManager::nextEgressHandle() noexcept {
  return getEgressHandle(self_.uni);
}

WebTransport::BidiStreamHandle WtStreamManager::nextBidiHandle() noexcept {
  return getBidiHandle(self_.bidi);
}

/**
 * ReadHandle & WriteHandle implementations here
 */
folly::SemiFuture<ReadHandle::StreamData> ReadHandle::readStreamData() {
  XLOG(FATAL) << "not implemented";
}

folly::Expected<folly::Unit, ReadHandle::ErrCode> ReadHandle::stopSending(
    uint32_t error) {
  XLOG(FATAL) << "not implemented";
}

folly::Expected<WriteHandle::FcState, ReadHandle::ErrCode>
WriteHandle::writeStreamData(
    std::unique_ptr<folly::IOBuf> data,
    bool fin,
    WebTransport::ByteEventCallback* byteEventCallback) {
  XLOG(FATAL) << "not implemented";
}
folly::Expected<folly::Unit, WriteHandle::ErrCode> WriteHandle::resetStream(
    uint32_t error) {
  XLOG(FATAL) << "not implemented";
}
folly::Expected<folly::Unit, WriteHandle::ErrCode> WriteHandle::setPriority(
    uint8_t level, uint32_t order, bool incremental) {
  XLOG(FATAL) << "not implemented";
}
folly::Expected<folly::SemiFuture<uint64_t>, WriteHandle::ErrCode>
WriteHandle::awaitWritable() {
  XLOG(FATAL) << "not implemented";
}

} // namespace proxygen::coro::detail
