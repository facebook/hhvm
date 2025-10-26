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

// TODO(@damlaj): change default fc
constexpr auto kDefaultFc = std::numeric_limits<uint16_t>::max();

struct BufferedData {
  folly::IOBuf chain; // head is always empty
  bool fin{false};
};

using namespace proxygen::coro::detail;
using ReadPromise = WtStreamManager::ReadPromise;
ReadPromise emptyReadPromise() {
  return ReadPromise::makeEmpty();
}

using StreamData = WtStreamManager::StreamData;

} // namespace

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
      peer_(peerNextStreams(dir, peer)),
      recv_(kDefaultFc),
      send_(kDefaultFc) {
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
  // nextExpectedStream can either be a self or peer stream; in either case, we
  // need to prevent exceeding concurrent streams limit
  NextStreams *next{&self_}, *limit{&peer_};
  if (isPeer(streamId)) {
    next = &peer_;
    limit = &self_;
  }

  uint64_t *nextId{&next->bidi}, *limitId{&limit->max.bidi};
  if (isUni(streamId)) {
    nextId = &next->uni;
    limitId = &limit->max.uni;
  }
  return (streamId == *nextId && ((streamId >> 2) < *limitId)) ? nextId
                                                               : nullptr;
}

bool WtStreamManager::onMaxStreams(MaxStreamsBidi bidi) {
  XCHECK_LE(bidi.maxStreams, kMaxVarint);
  bool valid = bidi.maxStreams >= peer_.max.bidi;
  peer_.max.bidi = std::max(peer_.max.bidi, bidi.maxStreams);
  return valid;
}

bool WtStreamManager::onMaxStreams(MaxStreamsUni uni) {
  XCHECK_LE(uni.maxStreams, kMaxVarint);
  bool valid = uni.maxStreams >= peer_.max.uni;
  peer_.max.uni = std::max(peer_.max.uni, uni.maxStreams);
  return valid;
}

struct ReadHandle : public WebTransport::StreamReadHandle {
  // why doesn't using StreamReadHandle::StreamReadHandle work here?
  ReadHandle(uint64_t id) : StreamReadHandle(id) {
  }
  using ErrCode = WebTransport::ErrorCode;
  // StreamReadHandle overrides
  folly::SemiFuture<StreamData> readStreamData() override;
  folly::Expected<folly::Unit, ErrCode> stopSending(uint32_t error) override;
  bool enqueue(StreamData&& data) noexcept;

  FlowController recv_{kDefaultFc};
  BufferedData buf_;
  ReadPromise promise_{emptyReadPromise()};
};
struct WriteHandle : public WebTransport::StreamWriteHandle {
  // why doesn't using StreamWriteHandle::StreamWriteHandle work here?
  WriteHandle(uint64_t id, BufferedFlowController& connSend)
      : StreamWriteHandle(id), connSend_(connSend) {
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

  WebTransport::StreamData dequeue(uint64_t atMost) noexcept;
  BufferedFlowController& connSend_;
  WtEgressContainer send_{kDefaultFc};
};

struct WtStreamManager::BidiHandle {
  WriteHandle wh;
  ReadHandle rh;

  BidiHandle(uint64_t id, BufferedFlowController& connSend)
      : wh(id, connSend), rh(id) {
  }

  static std::unique_ptr<BidiHandle> make(uint64_t id,
                                          BufferedFlowController& connSend) {
    return std::make_unique<BidiHandle>(id, connSend);
  }
};

WebTransport::StreamWriteHandle* WtStreamManager::getEgressHandle(
    uint64_t streamId) {
  if (!isEgress(streamId)) {
    return nullptr; // egress handle invalid
  }

  // create if next expected stream
  if (auto* next = nextExpectedStream(streamId)) {
    streams_.emplace(streamId, BidiHandle::make(streamId, send_));
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
    streams_.emplace(streamId, BidiHandle::make(streamId, send_));
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

bool WtStreamManager::onMaxData(MaxConnData data) noexcept {
  return send_.grant(data.maxData);
}
bool WtStreamManager::onMaxData(MaxStreamData data) noexcept {
  // TODO(@damlaj): connection-level err if not egress stream?
  auto* eh = static_cast<WriteHandle*>(getEgressHandle(data.streamId));
  return eh && eh->send_.grant(data.maxData);
}

bool WtStreamManager::enqueue(WtRh& rh, StreamData data) noexcept {
  auto len = data.data ? data.data->computeChainDataLength() : 0;
  bool err = !recv_.reserve(len);
  err = !err && (static_cast<ReadHandle&>(rh).enqueue(std::move(data)));
  return err;
}

StreamData WtStreamManager::dequeue(WtWh& wh, uint64_t atMost) noexcept {
  // we're limited by conn egress fc
  atMost = std::min(atMost, send_.getAvailable());
  // TODO(@damlaj): return len to elide unnecessarily computing chain len
  auto res = static_cast<WriteHandle&>(wh).dequeue(atMost);
  // commit len bytes to conn window
  send_.commit(res.data->computeChainDataLength());
  return res;
}

/**
 * ReadHandle & WriteHandle implementations here
 */
folly::SemiFuture<StreamData> ReadHandle::readStreamData() {
  // TODO(@damlaj): hook into interrupt handler, but somehow ensure no UB from
  // concurrent access
  XLOG_IF(FATAL, promise_.valid()) << "one pending read at a time";
  auto [p, f] = folly::makePromiseContract<StreamData>();
  if (!buf_.chain.empty() || buf_.fin) {
    // TODO(@damlaj): release flow control
    p.setValue(StreamData{buf_.chain.pop(), buf_.fin});
    return std::move(f);
  }
  promise_ = std::move(p);
  return std::move(f);
}

folly::Expected<folly::Unit, ReadHandle::ErrCode> ReadHandle::stopSending(
    uint32_t error) {
  XLOG(FATAL) << "not implemented";
}

bool ReadHandle::enqueue(StreamData&& data) noexcept {
  XCHECK(!buf_.fin) << "already rx'd eof";
  auto len = data.data ? data.data->computeChainDataLength() : 0;
  if (!recv_.reserve(len)) {
    return false; // error
  }
  if (len > 0) {
    buf_.chain.appendChain(std::move(data.data));
  }
  buf_.fin = data.fin;
  if (auto p = std::exchange(promise_, emptyReadPromise()); p.valid()) {
    // fulfill if pending promise; TODO(@damlaj): release fc
    p.setValue(StreamData{buf_.chain.pop(), buf_.fin});
  }
  return true;
}

folly::Expected<WriteHandle::FcState, WriteHandle::ErrCode>
WriteHandle::writeStreamData(std::unique_ptr<folly::IOBuf> data,
                             bool fin,
                             WebTransport::ByteEventCallback*) {
  // TODO(@damlaj): handle byte events & reset stream; elide unnecessarily
  // recomputing len
  auto len = data ? data->computeChainDataLength() : 0;
  connSend_.buffer(len);
  return send_.enqueue(std::move(data), fin) ? FcState::BLOCKED
                                             : FcState::UNBLOCKED;
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

// TODO(@damlaj): StreamData and DequeueResult should be the same struct
StreamData WriteHandle::dequeue(uint64_t atMost) noexcept {
  auto res = send_.dequeue(atMost);
  return StreamData{std::move(res.data), res.fin};
}

} // namespace proxygen::coro::detail
