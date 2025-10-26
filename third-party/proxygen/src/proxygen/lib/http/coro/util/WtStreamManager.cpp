/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/coro/util/WtStreamManager.h>

#include <folly/logging/xlog.h>

// fwd decls for Accessor
namespace {
struct ReadHandle;
struct WriteHandle;
}; // namespace

// grr Accessor needs to be a complete type before anon namespace below for
// ReadHandle & WriteHandle
namespace proxygen::coro::detail {

struct WtStreamManager::Accessor {
  Accessor(WtStreamManager& sm) : sm_(sm) {
  }
  void maybeGrantFc(ReadHandle* rh, uint64_t bytes) noexcept;
  void stopSending(ReadHandle& rh, uint32_t err) noexcept;
  void resetStream(WriteHandle& wh, uint32_t err) noexcept;
  void onStreamWritable(WriteHandle& wh) noexcept;
  // invoked when write handle or read handle are done
  void done(WriteHandle& wh) noexcept;
  void done(ReadHandle& rh) noexcept;

  bool isIngress(uint64_t id) {
    return sm_.isIngress(id);
  }
  bool isEgress(uint64_t id) {
    return sm_.isEgress(id);
  }
  auto& connSend() {
    return sm_.send_;
  }
  auto& writableStreams() {
    return sm_.writableStreams_;
  }
  WtStreamManager& sm_;
};

} // namespace proxygen::coro::detail

namespace {

constexpr uint8_t kStreamIdInc = 0x04;

// TODO(@damlaj): change default fc
constexpr auto kDefaultFc = std::numeric_limits<uint16_t>::max();

struct BufferedData {
  folly::IOBuf chain; // head is always empty
  bool fin{false};
};

size_t computeChainLength(std::unique_ptr<folly::IOBuf>& buf) {
  return buf ? buf->computeChainDataLength() : 0;
}

using namespace proxygen::coro;
using namespace proxygen::coro::detail;
using ReadPromise = WtStreamManager::ReadPromise;
ReadPromise emptyReadPromise() {
  return ReadPromise::makeEmpty();
}

using WritePromise = folly::Promise<uint64_t>;
WritePromise emptyWritePromise() {
  return WritePromise::makeEmpty();
}

using WtException = WtStreamManager::WtException;
using StreamData = WtStreamManager::StreamData;
using WebTransport = proxygen::WebTransport;
using Accessor = WtStreamManager::Accessor;
enum HandleState : uint8_t { Closed = 0, Open = 1 };

struct ReadHandle : public WebTransport::StreamReadHandle {
  // why doesn't using StreamReadHandle::StreamReadHandle work here?
  ReadHandle(uint64_t id, Accessor acc) noexcept;
  using ErrCode = WebTransport::ErrorCode;
  // StreamReadHandle overrides
  folly::SemiFuture<StreamData> readStreamData() override;
  folly::Expected<folly::Unit, ErrCode> stopSending(uint32_t error) override;
  bool enqueue(StreamData&& data) noexcept;
  void cancel(folly::exception_wrapper ex) noexcept;
  ReadPromise resetPromise() noexcept;
  void finish(bool done) noexcept;

  Accessor acc_;
  FlowController recv_{kDefaultFc};
  BufferedData buf_;
  ReadPromise promise_{emptyReadPromise()};
  folly::exception_wrapper ex_; // set on self stop sending or peer reset_stream
  HandleState state_;
  WriteHandle* wh_{nullptr}; // ptr to the symmetric wh
};
struct WriteHandle : public WebTransport::StreamWriteHandle {
  // why doesn't using StreamWriteHandle::StreamWriteHandle work here?
  WriteHandle(uint64_t id, Accessor acc) noexcept;
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
  bool onMaxData(uint64_t offset);
  WritePromise resetPromise() noexcept;
  void cancel(folly::exception_wrapper ex) noexcept;
  void finish(bool done) noexcept;

  Accessor acc_;
  WtEgressContainer send_{kDefaultFc};
  folly::exception_wrapper ex_; // set on peer stop_sending or self reset_stream
  uint64_t err{kInvalidVarint};
  WritePromise promise_{emptyWritePromise()};
  HandleState state_;
  ReadHandle* rh_{nullptr}; // ptr to the symmetric rh
};

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

/**
 * - if wh == nullptr:  only release connection-level flow control (e.g.
 *   http/3 doesn't utilize wt stream-level fc)
 *
 * - otherwise release both connection- and stream-level flow control
 */
void Accessor::maybeGrantFc(ReadHandle* rh, uint64_t bytes) noexcept {
  // TODO(@damlaj): user-defined flow control values and release when 50% is
  // read
  if (rh) { // handle stream-level flow control
    auto& streamRecv = rh->recv_;
    // if peer has less than 32KB to send, issue fc
    bool issueFc = streamRecv.getAvailable() < (kDefaultFc / 2);
    if (issueFc) {
      streamRecv.grant(streamRecv.getCurrentOffset() + kDefaultFc);
      sm_.enqueueEvent(MaxStreamData{{streamRecv.getMaxOffset()}, rh->getID()});
    }
  }
  auto& connRecv = sm_.recv_;
  // if peer has less than 32KB to send, issue fc
  bool issueFc = connRecv.getAvailable() < (kDefaultFc / 2);
  if (issueFc) {
    connRecv.grant(connRecv.getCurrentOffset() + kDefaultFc);
    sm_.enqueueEvent(MaxConnData{connRecv.getMaxOffset()});
  }
}

void Accessor::stopSending(ReadHandle& rh, uint32_t err) noexcept {
  sm_.enqueueEvent(StopSending{rh.getID(), err});
}

void Accessor::resetStream(WriteHandle& wh, uint32_t err) noexcept {
  sm_.enqueueEvent(ResetStream{wh.getID(), err});
}

void Accessor::onStreamWritable(WriteHandle& wh) noexcept {
  sm_.onStreamWritable(wh);
}

void Accessor::done(WriteHandle& wh) noexcept {
  XCHECK_EQ(wh.state_, Closed);
  if (wh.rh_->state_ == Closed) { // bidi done
    sm_.streams_.erase(wh.getID());
  }
}

void Accessor::done(ReadHandle& rh) noexcept {
  XCHECK_EQ(rh.state_, Closed);
  if (rh.wh_->state_ == Closed) { // bidi done
    sm_.streams_.erase(rh.getID());
  }
}

WtStreamManager::WtStreamManager(WtDir dir,
                                 WtMaxStreams self,
                                 WtMaxStreams peer,
                                 Callback& cb)
    : dir_(dir),
      self_(selfNextStreams(dir, self)),
      peer_(peerNextStreams(dir, peer)),
      recv_(kDefaultFc),
      send_(kDefaultFc),
      cb_(cb) {
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
  if (drain_) {
    return nullptr;
  }

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

void WtStreamManager::enqueueEvent(Event&& ev) noexcept {
  bool wasEmpty = !hasEvent();
  events_.push_back(std::move(ev));
  if (wasEmpty && hasEvent()) {
    cb_.eventsAvailable();
  }
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

struct WtStreamManager::BidiHandle {
  WriteHandle wh;
  ReadHandle rh;

  BidiHandle(uint64_t id, WtStreamManager& sm)
      : wh(id, Accessor{sm}), rh(id, Accessor{sm}) {
    wh.rh_ = &rh;
    rh.wh_ = &wh;
  }

  static std::unique_ptr<BidiHandle> make(uint64_t id, WtStreamManager& sm) {
    return std::make_unique<BidiHandle>(id, sm);
  }
};

WebTransport::StreamWriteHandle* WtStreamManager::getEgressHandle(
    uint64_t streamId) {
  if (!isEgress(streamId)) {
    return nullptr; // egress handle invalid
  }

  // create if next expected stream
  if (auto* next = nextExpectedStream(streamId)) {
    streams_.emplace(streamId, BidiHandle::make(streamId, *this));
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
    streams_.emplace(streamId, BidiHandle::make(streamId, *this));
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
  bool wasWritable = nextWritable() != nullptr;
  send_.grant(data.maxData); // TODO(@damlaj): handle ::grant err
  if (!wasWritable && nextWritable()) {
    cb_.eventsAvailable();
  }
  return true;
}

bool WtStreamManager::onMaxData(MaxStreamData data) noexcept {
  // TODO(@damlaj): connection-level err if not egress stream?
  auto* eh = static_cast<WriteHandle*>(getEgressHandle(data.streamId));
  return eh && eh->onMaxData(data.maxData);
}

bool WtStreamManager::onStopSending(StopSending data) noexcept {
  if (auto* eh = static_cast<WriteHandle*>(getEgressHandle(data.streamId))) {
    auto ex = folly::make_exception_wrapper<WtException>(uint32_t(data.err),
                                                         "rx stop_sending");
    eh->cancel(std::move(ex));
    return true;
  }
  return false;
}

bool WtStreamManager::onResetStream(ResetStream data) noexcept {
  if (auto* rh = static_cast<ReadHandle*>(getIngressHandle(data.streamId))) {
    auto ex = folly::make_exception_wrapper<WtException>(uint32_t(data.err),
                                                         "rx reset_stream");
    rh->cancel(std::move(ex));
    return true;
  }
  return false;
}

void WtStreamManager::onDrainSession(DrainSession) noexcept {
  drain();
}

void WtStreamManager::onCloseSession(CloseSession close) noexcept {
  drain();
  auto ex = folly::make_exception_wrapper<WtException>(close.err, close.msg);
  auto streams = std::move(streams_);
  for (auto& [_, handle] : streams) {
    handle->rh.cancel(ex);
    handle->wh.cancel(ex);
  }
}

bool WtStreamManager::enqueue(WtRh& rh, StreamData data) noexcept {
  auto len = computeChainLength(data.data);
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
  send_.commit(computeChainLength(res.data));
  return res;
}

WtStreamManager::WtWh* WtStreamManager::nextWritable() const noexcept {
  bool hasSend = !writableStreams_.empty() && send_.getAvailable() > 0;
  return hasSend ? *writableStreams_.begin() : nullptr;
}

void WtStreamManager::onStreamWritable(WtWh& wh) noexcept {
  bool wasEmpty = !hasEvent();
  writableStreams_.insert(&wh);
  if (wasEmpty && hasEvent()) {
    cb_.eventsAvailable();
  }
}

void WtStreamManager::drain() noexcept {
  drain_ = true;
}

void WtStreamManager::shutdown(CloseSession data) noexcept {
  onCloseSession(data);
  enqueueEvent(std::move(data));
}

bool WtStreamManager::Compare::operator()(const WtWh* l, const WtWh* r) const {
  return l->getID() < r->getID();
}

bool WtStreamManager::hasEvent() const noexcept {
  return nextWritable() != nullptr || !events_.empty();
}

} // namespace proxygen::coro::detail

/**
 * ReadHandle & WriteHandle implementations here
 */

ReadHandle::ReadHandle(uint64_t id, Accessor acc) noexcept
    : StreamReadHandle(id),
      acc_(acc),
      state_(static_cast<HandleState>(acc.isIngress(id))) {
}

folly::SemiFuture<StreamData> ReadHandle::readStreamData() {
  // TODO(@damlaj): hook into interrupt handler, but somehow ensure no UB from
  // concurrent access
  XLOG_IF(FATAL, promise_.valid()) << "one pending read at a time";
  auto [p, f] = folly::makePromiseContract<StreamData>();
  if (!buf_.chain.empty() || buf_.fin) {
    auto len = buf_.chain.computeChainDataLength();
    // only issue conn-level fc if we've rx'd fin
    acc_.maybeGrantFc(buf_.fin ? nullptr : this, len);
    p.setValue(StreamData{buf_.chain.pop(), buf_.fin});
    finish(buf_.fin);
    return std::move(f);
  }
  // always deliver buffered data (even if rx fin) prior to delivering err
  if (ex_) {
    p.setException(ex_);
    finish(/*done=*/true);
    return std::move(f);
  }
  promise_ = std::move(p);
  return std::move(f);
}

folly::Expected<folly::Unit, ReadHandle::ErrCode> ReadHandle::stopSending(
    uint32_t error) {
  acc_.stopSending(*this, error);
  // **beware cancel must be last** (`this` can be deleted immediately after)
  cancel(folly::make_exception_wrapper<WtException>(error, "tx stop_sending"));
  return folly::unit;
}

bool ReadHandle::enqueue(StreamData&& data) noexcept {
  XCHECK(!buf_.fin) << "already rx'd eof";
  auto len = computeChainLength(data.data);
  if (!recv_.reserve(len)) {
    return false; // error
  }
  if (len > 0) {
    buf_.chain.appendChain(std::move(data.data));
  }
  buf_.fin = data.fin;
  if (auto p = resetPromise(); p.valid()) {
    // only issue conn-level fc if we've rx'd fin
    acc_.maybeGrantFc(buf_.fin ? nullptr : this, len);
    p.setValue(StreamData{buf_.chain.pop(), buf_.fin});
    finish(buf_.fin);
  }
  return true;
}

void ReadHandle::cancel(folly::exception_wrapper ex) noexcept {
  ex_ = std::move(ex);
  // any pending reads should be resolved with ex
  if (auto p = resetPromise(); p.valid()) {
    p.setException(ex_);
  }
  cs_.requestCancellation();
  finish(/*done=*/true);
}

ReadPromise ReadHandle::resetPromise() noexcept {
  return std::exchange(promise_, emptyReadPromise());
}

void ReadHandle::finish(bool done) noexcept {
  if (done) {
    state_ = Closed;
    acc_.done(*this);
  }
}

WriteHandle::WriteHandle(uint64_t id, Accessor acc) noexcept
    : StreamWriteHandle(id),
      acc_(acc),
      state_(static_cast<HandleState>(acc.isEgress(id))) {
}

folly::Expected<WriteHandle::FcState, WriteHandle::ErrCode>
WriteHandle::writeStreamData(std::unique_ptr<folly::IOBuf> data,
                             bool fin,
                             WebTransport::ByteEventCallback*) {
  // TODO(@damlaj): handle byte events & reset stream; elide unnecessarily
  // recomputing len
  auto len = computeChainLength(data);
  XLOG_IF(ERR, !(len || fin)) << "no-op writeStreamData";
  acc_.connSend().buffer(len);
  auto res = send_.enqueue(std::move(data), fin) ? FcState::BLOCKED
                                                 : FcState::UNBLOCKED;
  if (send_.canSendData()) {
    acc_.onStreamWritable(*this); // stream is now writable
  }
  return res;
}

folly::Expected<folly::Unit, WriteHandle::ErrCode> WriteHandle::resetStream(
    uint32_t error) {
  acc_.resetStream(*this, error);
  // **beware cancel must be last** (`this` can be deleted immediately after)
  cancel(folly::make_exception_wrapper<WtException>(error, "tx reset_stream"));
  return folly::unit;
}

folly::Expected<folly::Unit, WriteHandle::ErrCode> WriteHandle::setPriority(
    uint8_t level, uint32_t order, bool incremental) {
  XLOG(FATAL) << "not implemented";
}

bool WriteHandle::onMaxData(uint64_t offset) {
  // TODO(@damlaj): handle ::grant error
  send_.grant(offset);
  if (send_.canSendData()) {
    acc_.writableStreams().insert(this); // stream is now writable
  }
  return true;
}

folly::Expected<folly::SemiFuture<uint64_t>, WriteHandle::ErrCode>
WriteHandle::awaitWritable() {
  XCHECK(!promise_.valid()) << "at most one pending awaitWritable";
  auto [p, f] = folly::makePromiseContract<uint64_t>();
  const auto bufferAvailable = send_.window().getBufferAvailable();
  if (bufferAvailable > 0) {
    p.setValue(bufferAvailable);
    return std::move(f);
  }
  promise_ = std::move(p);
  return std::move(f);
}

WritePromise WriteHandle::resetPromise() noexcept {
  return std::exchange(promise_, emptyWritePromise());
}

// TODO(@damlaj): StreamData and DequeueResult should be the same struct
StreamData WriteHandle::dequeue(uint64_t atMost) noexcept {
  XCHECK_NE(state_, Closed) << "dequeue after close";
  auto res = send_.dequeue(atMost);
  const auto bufferAvailable = send_.window().getBufferAvailable();
  if (bufferAvailable > 0) {
    if (auto p = resetPromise(); p.valid()) {
      p.setValue(bufferAvailable);
    }
  }
  if (!send_.canSendData()) {
    acc_.writableStreams().erase(this);
  }
  finish(res.fin);
  return StreamData{std::move(res.data), res.fin};
}

void WriteHandle::cancel(folly::exception_wrapper ex) noexcept {
  ex_ = std::move(ex);
  // any pending awaitWritable should be resolved with ex
  if (auto p = resetPromise(); p.valid()) {
    p.setException(ex_);
  }
  acc_.writableStreams().erase(this);
  cs_.requestCancellation();
  // **beware finish must be last** (`this` can be deleted immediately after)
  finish(/*done=*/true);
}

void WriteHandle::finish(bool done) noexcept {
  if (done) {
    state_ = Closed;
    acc_.done(*this);
  }
}
