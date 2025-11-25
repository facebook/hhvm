/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * This comment block documents the implementation details of WtStreamManager
 * (i.e. the how).
 *
 * WriteHandle & ReadHandle (concrete implementations of
 * WebTransport::WriteHandle and WebTransport::ReadHandle respsectively, defined
 * below) are anonymous and strictly scoped to this TU. This requires
 * downcasting in all apis that receive a pointer to those pure virtual classes.
 * To prevent leaking implementation details by marking methods in
 * WtStreamManager public just for internal use by WriteHandle & ReadHandle, we
 * utilize a friend struct Accessor (also anonymously defined and strictly
 * scoped to this TU).
 *
 * First thing to note is that we always allocate both a WriteHandle and
 * ReadHandle regardless of the underlying properities of the stream (e.g. if
 * it's a unidirectional stream). This makes things extremely easier to reason
 * about, as there's a single map from [id]->[bidi_handle]. The public api in
 * WtStreamManager is extremely restrictive, and we do not hand out a pointer to
 * an invalid handle. For example, if a client WtStreamManager attempts to
 * retrieve a EgressHandle for a server-initiated unidirectional stream (e.g.
 * id=0x03), the state for such a handle exists but we return nullptr – it's
 * effectively invisible to the user.
 *
 * A note about flow control:
 * – Ingress flow control is strict, as a peer exceeding the advertised max
 *   offset is a connection- or stream-level flow control.
 *
 * – Egress flow control is not strict, as an application can enqueue too much
 *   data for any number of reasons (large write, ignoring backpressure, etc.).
 *   We buffer this data in WriteHandle and is subsequently dequeued by the
 *   transport whenever the stream is writable (hence the need for
 *   WtEgressContainer to manage this small complexity). Applications should
 *   respect backpressure signalled by returning FcState::BLOCKED from
 *   WriteHandle::writeStreamData
 *
 * A note about stream states:
 * – An egress handle transitions from [HandleState::Open] ->
 *   [HandleState::Closed] in three cases: fin is dequeued from WriteHandle via
 *   ::dequeue, the application resets the stream (i.e.
 *   WebTransport::WriteHandle::resetStream), or the transport receives a
 *   stop_sending and WtStreamManager::onStopSending is invoked.
 *
 * – An ingress handle transitions from [HandleState::Open] ->
 *   [HandleState::Closed] in three cases: fin is read via ::readStreamData(),
 *   the application is no longer interested in ingress (i.e.
 *   WebTransport::ReadHandle::stopSending), or the transport receives a
 *   reset_stream and WtStreamManager::onResetStream is invoked.
 *
 * – An invalid handle (e.g. client egress handle for a
 *   server-initiated unidirectional stream) starts in the HandleState::Closed.
 *
 * – Once both the ingress & egress handles have reached the HandleState::Closed
 *   state, we deallocate the state. Since we always allocate both a ReadHandle
 *   and WriteHandle, they're both unconditionally linked together for
 *   simplicity.
 */

#include <proxygen/lib/http/webtransport/WtStreamManager.h>

#include <folly/logging/xlog.h>

// fwd decls for Accessor
namespace {
struct ReadHandle;
struct WriteHandle;

// helpers to reduce the repetitive static_cast
#define readhandle_ptr_cast(ptr) (static_cast<ReadHandle*>(ptr))
#define writehandle_ptr_cast(ptr) (static_cast<WriteHandle*>(ptr))
#define readhandle_ref_cast(ptr) (static_cast<ReadHandle&>(ptr))
#define writehandle_ref_cast(ptr) (static_cast<WriteHandle&>(ptr))

}; // namespace

namespace proxygen::coro::detail {

// Accessor needs to be a complete type before anon namespace below for
// ReadHandle & WriteHandle. This struct is a friend of WtStreamManager and
// consequently has access to private members/functions.
struct WtStreamManager::Accessor {
  explicit Accessor(WtStreamManager& sm) : sm_(sm) {
  }
  void maybeGrantFc(ReadHandle* rh, uint64_t bytesRead) noexcept;
  void stopSending(ReadHandle& rh, uint32_t err) noexcept;
  void resetStream(WriteHandle& wh,
                   uint32_t err,
                   uint64_t reliableSize) noexcept;
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
    return sm_.connSendFc_;
  }
  auto& writableStreams() {
    return sm_.writableStreams_;
  }
  WtStreamManager& sm_;
};

} // namespace proxygen::coro::detail

namespace {

constexpr uint8_t kStreamIdInc = 0x04;

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
using Result = WtStreamManager::Result;
using StreamData = WtStreamManager::StreamData;
using WebTransport = proxygen::WebTransport;
using Accessor = WtStreamManager::Accessor;
enum HandleState : uint8_t { Closed = 0, Open = 1 };

struct ReadHandle : public WebTransport::StreamReadHandle {
  // why doesn't using StreamReadHandle::StreamReadHandle work here?
  ReadHandle(uint64_t id, uint64_t initRecvWnd, Accessor acc) noexcept;
  using ErrCode = WebTransport::ErrorCode;
  // StreamReadHandle overrides
  folly::SemiFuture<StreamData> readStreamData() override;
  folly::Expected<folly::Unit, ErrCode> stopSending(uint32_t error) override;
  Result enqueue(StreamData&& data) noexcept;
  void cancel(folly::exception_wrapper ex,
              uint64_t reliableSize = kInvalidVarint) noexcept;
  ReadPromise resetPromise() noexcept;
  void finish(bool done) noexcept;

  Accessor smAccessor_;
  FlowController streamRecvFc_;
  BufferedData ingress_;
  uint64_t bytesRead_{0};
  ReadPromise promise_{emptyReadPromise()};
  HandleState state_;
  WriteHandle* wh_{nullptr}; // ptr to the symmetric wh
};
struct WriteHandle : public WebTransport::StreamWriteHandle {
  WriteHandle(uint64_t id, uint64_t initSendWnd, Accessor acc) noexcept;
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
  Result onMaxData(uint64_t offset);
  WritePromise resetPromise() noexcept;
  void cancel(folly::exception_wrapper ex) noexcept;
  void finish(bool done) noexcept;

  Accessor smAccessor_;
  WtBufferedStreamData bufferedSendData_;
  uint64_t err{kInvalidVarint};
  WritePromise promise_{emptyWritePromise()};
  HandleState state_;
  ReadHandle* rh_{nullptr}; // ptr to the symmetric rh
};

} // namespace

namespace proxygen::coro::detail {

/**
 * If rh == nullptr, ::maybeGrantFc only releases connection-level flow control.
 * This is set to nullptr if a fin/reset_stream have been received on a
 * stream, as releasing stream-level flow control is no longer
 * necessary/reasonable.
 *
 * Otherwise release both connection- and stream-level flow control
 */
void Accessor::maybeGrantFc(ReadHandle* rh, uint64_t bytesRead) noexcept {
  // TODO(@damlaj): user-defined flow control values and release when 50% is
  // read
  XLOG(DBG6) << __func__ << "; rh=" << rh;
  if (rh) { // handle stream-level flow control
    uint64_t initStreamRecvFc = sm_.initStreamRecvFc(rh->getID());
    auto& streamRecv = rh->streamRecvFc_;
    // if peer has less than rwnd / 2 to send, issue fc
    bool issueFc = streamRecv.getAvailable() <= (initStreamRecvFc / 2);
    XLOG(DBG6) << __func__ << "avail=" << streamRecv.getAvailable()
               << "; issue=" << issueFc;
    if (issueFc) {
      streamRecv.grant(streamRecv.getCurrentOffset() + initStreamRecvFc);
      sm_.enqueueEvent(MaxStreamData{{streamRecv.getMaxOffset()}, rh->getID()});
    }
  }
  uint64_t initConnRecvFc = sm_.wtConfig_.selfMaxConnData;
  // if peer has less than rwnd / 2 to send, issue fc
  auto& connRecv = sm_.connRecvFc_;
  sm_.connBytesRead_ += bytesRead;
  bool issueFc =
      (connRecv.getMaxOffset() - sm_.connBytesRead_) <= (initConnRecvFc / 2);
  XLOG(DBG6) << __func__ << "; connMaxOffset=" << connRecv.getMaxOffset()
             << "; connBytesRead_" << sm_.connBytesRead_
             << "; issue=" << issueFc;
  if (issueFc) {
    connRecv.grant(connRecv.getCurrentOffset() + initConnRecvFc);
    sm_.enqueueEvent(MaxConnData{connRecv.getMaxOffset()});
  }
}

void Accessor::stopSending(ReadHandle& rh, uint32_t err) noexcept {
  sm_.enqueueEvent(StopSending{rh.getID(), err});
}

void Accessor::resetStream(WriteHandle& wh,
                           uint32_t err,
                           uint64_t reliableSize) noexcept {
  sm_.enqueueEvent(ResetStream{wh.getID(), err, reliableSize});
}

void Accessor::onStreamWritable(WriteHandle& wh) noexcept {
  sm_.onStreamWritable(wh);
}

void Accessor::done(WriteHandle& wh) noexcept {
  XCHECK_EQ(wh.state_, Closed);
  if (wh.rh_->state_ == Closed) { // bidi done
    sm_.erase(wh.getID());
  }
}

void Accessor::done(ReadHandle& rh) noexcept {
  XCHECK_EQ(rh.state_, Closed);
  if (rh.wh_->state_ == Closed) { // bidi done
    sm_.erase(rh.getID());
  }
}

/*static*/ auto WtStreamManager::nextStreamIds(WtDir dir) noexcept
    -> NextStreamIds {
  return dir == Client ? NextStreamIds{.bidi = 0x00, .uni = 0x02}
                       : NextStreamIds{.bidi = 0x01, .uni = 0x03};
}

/**
 * MaxStreams (and StreamsCounter) is indexed by the *type* of the stream, which
 * is derived from the least two significant bits of the stream id, as per
 * rfc9000 (i.e. the array must be created in the following order: client bidi
 * (0x00), server bidi (0x01), client uni (0x02), server uni (0x03))
 */
/*static*/ auto WtStreamManager::maxStreams(WtDir dir,
                                            const WtConfig& config) noexcept
    -> MaxStreamsContainer::Type {
  MaxStreamsContainer::Type res{config.peerMaxStreamsBidi,
                                config.selfMaxStreamsBidi,
                                config.peerMaxStreamsUni,
                                config.selfMaxStreamsUni};
  if (dir == Server) {
    res = {config.selfMaxStreamsBidi,
           config.peerMaxStreamsBidi,
           config.selfMaxStreamsUni,
           config.peerMaxStreamsUni};
  }
  return res;
}

/*static*/ auto WtStreamManager::streamType(uint64_t streamId) noexcept
    -> StreamType {
  return static_cast<StreamType>(streamId & 0b11);
}

auto WtStreamManager::StreamsCounterContainer::getCounter(
    uint64_t streamId) noexcept -> StreamsCounter& {
  return streamsCounter_[streamType(streamId)];
}

auto WtStreamManager::StreamsCounterContainer::getCounter(
    uint64_t streamId) const noexcept -> const StreamsCounter& {
  return streamsCounter_[streamType(streamId)];
}

WtStreamManager::MaxStreamsContainer::MaxStreamsContainer(
    Type maxStreams) noexcept
    : maxStreams_(maxStreams) {
}

uint64_t WtStreamManager::MaxStreamsContainer::getMaxStreams(
    uint64_t streamId) const noexcept {
  return maxStreams_[streamType(streamId)];
}

uint64_t& WtStreamManager::MaxStreamsContainer::getMaxStreams(
    uint64_t streamId) noexcept {
  return maxStreams_[streamType(streamId)];
}

WtStreamManager::WtStreamManager(WtDir dir,
                                 const WtConfig& config,
                                 EgressCallback& egressCb,
                                 IngressCallback& ingressCb) noexcept
    : dir_(dir),
      nextStreamIds_(nextStreamIds(dir)),
      maxStreams_(maxStreams(dir, config)),
      wtConfig_(config),
      connRecvFc_(config.selfMaxConnData),
      connSendFc_(config.peerMaxConnData),
      egressCb_(egressCb),
      ingressCb_(ingressCb) {
  XCHECK(dir <= WtDir::Server) << "invalid dir";
}

WtStreamManager::~WtStreamManager() noexcept = default;

bool WtStreamManager::isSelf(uint64_t streamId) const {
  return (streamId & 0x01) == uint8_t(dir_);
}
bool WtStreamManager::isPeer(uint64_t streamId) const {
  return !isSelf(streamId);
}

bool WtStreamManager::isUni(uint64_t streamId) const {
  return (streamId & 0x02) != 0;
}
bool WtStreamManager::isBidi(uint64_t streamId) const {
  return !isUni(streamId);
}

bool WtStreamManager::isIngress(uint64_t streamId) const {
  return isBidi(streamId) || isPeer(streamId);
}
bool WtStreamManager::isEgress(uint64_t streamId) const {
  return isBidi(streamId) || isSelf(streamId);
}

/**
 * Enqueues an event into events. If this is the first event to be enqueued, it
 * will invoke the Callback (edge-triggered).
 */
void WtStreamManager::enqueueEvent(Event&& ev) noexcept {
  bool wasEmpty = !hasEvent();
  ctrlEvents_.push_back(std::move(ev));
  if (wasEmpty && hasEvent()) {
    egressCb_.eventsAvailable();
  }
}

WtStreamManager::Result WtStreamManager::onMaxStreams(MaxStreamsBidi bidi) {
  XCHECK_LE(bidi.maxStreams, kMaxVarint);
  // the "StreamType" is derived from a self bidi id, simply use nextStreamIds_
  auto& maxBidi = maxStreams_.getMaxStreams(nextStreamIds_.bidi);
  bool valid = bidi.maxStreams >= maxBidi;
  maxBidi = std::max(maxBidi, bidi.maxStreams);
  return (Result)valid;
}

WtStreamManager::Result WtStreamManager::onMaxStreams(MaxStreamsUni uni) {
  XCHECK_LE(uni.maxStreams, kMaxVarint);
  // the "StreamType" is derived from a self uni id, simply use nextStreamIds_
  auto& maxUni = maxStreams_.getMaxStreams(nextStreamIds_.uni);
  bool valid = uni.maxStreams >= maxUni;
  maxUni = std::max(maxUni, uni.maxStreams);
  return (Result)valid;
}

struct WtStreamManager::BidiHandle {
  WriteHandle wh;
  ReadHandle rh;

  BidiHandle(uint64_t id, WtStreamManager& sm)
      : wh(id, sm.initStreamSendFc(id), Accessor{sm}),
        rh(id, sm.initStreamRecvFc(id), Accessor{sm}) {
    // link ReadHandle<->WriteHandle
    wh.rh_ = &rh;
    rh.wh_ = &wh;
    if (sm.isPeer(id)) {
      /**
       * NOTE: & TODO: This is intentionally invoked now, before the stream is
       * inserted into the map. This is to prevent the case where an application
       * bidirectionally resets a stream inline (from
       * IngressCallback::onNewPeerStream) and causes its subsequent
       * deallocation while the handle is returned to the backing transport
       * (e.g. the ::getOrCreateBidiHandle invocation that caused this stream
       * allocation in the first place).
       *
       * The backing transport must accumulate these streams and deliver them to
       * the application in the next EventBase loop. The more appropriate fix
       * here is for the containing class to pass in a folly::Executor to
       * StreamManager, allowing WtStreamManager to defer invoking
       * ::onNewPeerStream (effectively asynchronous w.r.t stream allocation).
       */
      sm.ingressCb_.onNewPeerStream(id);
    }
  }

  static std::unique_ptr<BidiHandle> make(uint64_t id, WtStreamManager& sm) {
    return std::make_unique<BidiHandle>(id, sm);
  }
};

bool WtStreamManager::streamLimitExceeded(uint64_t streamId) const noexcept {
  uint64_t opened = streamsCounter_.getCounter(streamId).opened;
  uint64_t limit = maxStreams_.getMaxStreams(streamId);
  bool exceeded = opened >= limit;
  XLOG_IF(ERR, exceeded) << __func__ << "; opened=" << opened
                         << "; limit=" << limit << "; id=" << streamId;
  return exceeded;
}

WtStreamManager::BidiHandle* WtStreamManager::getOrCreateBidiHandleImpl(
    uint64_t streamId) noexcept {
  auto it = streams_.find(streamId);
  if (it != streams_.end()) {
    return it->second.get();
  }
  // prevent new streams if shutdown or either peer/self limit saturated
  if (streamLimitExceeded(streamId) || shutdown_) {
    return nullptr;
  }
  streamsCounter_.getCounter(streamId).opened++;
  it = streams_.emplace(streamId, BidiHandle::make(streamId, *this)).first;
  return it->second.get();
}

WebTransport::StreamWriteHandle* WtStreamManager::getOrCreateEgressHandle(
    uint64_t streamId) noexcept {
  auto* handle =
      isEgress(streamId) ? getOrCreateBidiHandleImpl(streamId) : nullptr;
  return handle ? &handle->wh : nullptr;
}

WebTransport::StreamReadHandle* WtStreamManager::getOrCreateIngressHandle(
    uint64_t streamId) noexcept {
  auto* handle =
      isIngress(streamId) ? getOrCreateBidiHandleImpl(streamId) : nullptr;
  return handle ? &handle->rh : nullptr;
}

WebTransport::BidiStreamHandle WtStreamManager::getOrCreateBidiHandle(
    uint64_t streamId) noexcept {
  WebTransport::BidiStreamHandle res{nullptr, nullptr};
  if (auto* handle = getOrCreateBidiHandleImpl(streamId)) {
    res.readHandle = isIngress(streamId) ? &handle->rh : nullptr;
    res.writeHandle = isEgress(streamId) ? &handle->wh : nullptr;
  }
  return res;
}

WtStreamManager::WtWriteHandle* WtStreamManager::createEgressHandle() noexcept {
  auto* handle = getOrCreateEgressHandle(nextStreamIds_.uni);
  nextStreamIds_.uni += (handle ? kStreamIdInc : 0);
  return handle;
}

WebTransport::BidiStreamHandle WtStreamManager::createBidiHandle() noexcept {
  auto handle = getOrCreateBidiHandle(nextStreamIds_.bidi);
  if (handle.readHandle || handle.writeHandle) {
    nextStreamIds_.bidi += kStreamIdInc;
  }
  return handle;
}

WtStreamManager::Result WtStreamManager::onMaxData(MaxConnData data) noexcept {
  XLOG(DBG9) << __func__ << " maxData=" << data.maxData;
  if (!connSendFc_.grant(data.maxData)) {
    return Fail;
  }
  bool wasEmpty = !hasEvent();
  if (!wasEmpty && hasEvent()) {
    egressCb_.eventsAvailable();
  }
  return Ok;
}

WtStreamManager::Result WtStreamManager::onMaxData(
    MaxStreamData data) noexcept {
  // TODO(@damlaj): connection-level err if not egress stream?
  XLOG(DBG9) << __func__ << "; id=" << data.streamId
             << "; maxData=" << data.maxData;
  auto* eh = writehandle_ptr_cast(getOrCreateEgressHandle(data.streamId));
  return (eh && eh->onMaxData(data.maxData)) ? Ok : Fail;
}

WtStreamManager::Result WtStreamManager::onStopSending(
    StopSending data) noexcept {
  XLOG(DBG9) << __func__ << "; id=" << data.streamId << "; err=" << data.err;
  if (auto* eh = writehandle_ptr_cast(getOrCreateEgressHandle(data.streamId))) {
    auto ex = folly::make_exception_wrapper<WtException>(uint32_t(data.err),
                                                         "rx stop_sending");
    eh->cancel(std::move(ex));
    return Ok;
  }
  return Fail;
}

WtStreamManager::Result WtStreamManager::onResetStream(
    ResetStream data) noexcept {
  XLOG(DBG9) << __func__ << "; id=" << data.streamId << "; err=" << data.err;
  if (auto* rh = readhandle_ptr_cast(getOrCreateIngressHandle(data.streamId))) {
    auto ex = folly::make_exception_wrapper<WtException>(uint32_t(data.err),
                                                         "rx reset_stream");
    rh->cancel(std::move(ex), data.reliableSize);
    return Ok;
  }
  return Fail;
}

void WtStreamManager::onDrainSession(DrainSession) noexcept {
  drain_ = true;
}

void WtStreamManager::onCloseSession(CloseSession close) noexcept {
  shutdown_ = true;
  auto ex = folly::make_exception_wrapper<WtException>(close.err, close.msg);
  auto streams = std::move(streams_);
  for (auto& [_, handle] : streams) {
    handle->rh.cancel(ex);
    handle->wh.cancel(ex);
  }
}

Result WtStreamManager::enqueue(WtReadHandle& rh, StreamData data) noexcept {
  auto len = computeChainLength(data.data);
  bool err = !connRecvFc_.reserve(len);
  return (!err && (readhandle_ref_cast(rh).enqueue(std::move(data)))) ? Ok
                                                                      : Fail;
}

StreamData WtStreamManager::dequeue(WtWriteHandle& wh,
                                    uint64_t atMost) noexcept {
  // we're limited by conn egress fc
  atMost = std::min(atMost, connSendFc_.getAvailable());
  auto res = writehandle_ref_cast(wh).dequeue(atMost);
  // TODO(@damlaj): return len to elide unnecessarily computing chain len
  auto len = computeChainLength(res.data);
  // commit len bytes to conn window
  connSendFc_.commit(len);
  XLOG(DBG8) << __func__ << "; atMost=" << atMost << "; len=" << len
             << "; fin=" << res.fin;
  return res;
}

WtStreamManager::WtWriteHandle* WtStreamManager::nextWritable() const noexcept {
  WriteHandle* wh = !writableStreams_.empty()
                        ? writehandle_ptr_cast(*writableStreams_.begin())
                        : nullptr;
  // streams with only a pending fin should be yielded even if connection-level
  // flow control window is blocked
  return (wh && (connSendFc_.getAvailable() > 0 ||
                 wh->bufferedSendData_.onlyFinPending()))
             ? wh
             : nullptr;
}

void WtStreamManager::onStreamWritable(WtWriteHandle& wh) noexcept {
  bool wasEmpty = !hasEvent();
  writableStreams_.insert(&wh);
  if (wasEmpty && hasEvent()) {
    egressCb_.eventsAvailable();
  }
}

void WtStreamManager::drain() noexcept {
  drain_ = true;
  enqueueEvent(DrainSession{});
}

void WtStreamManager::shutdown(CloseSession data) noexcept {
  onCloseSession(data);
  enqueueEvent(std::move(data));
}

/**
 * Even if wt connection is flow-control blocked, a stream with only a fin
 * pending should be yielded from ::nextWritable. We insert streams with only a
 * pending fin first in the set for ::nextWritable to check such cases in O(1)
 * time.
 */
bool WtStreamManager::Compare::operator()(const WtWriteHandle* l,
                                          const WtWriteHandle* r) const {
  const auto* lWh = static_cast<const WriteHandle*>(l);
  const auto* rWh = static_cast<const WriteHandle*>(r);
  // safe to cast to int64_t as getID() is limited by kMaxVarint
  int64_t lId = lWh->getID();
  int64_t rId = rWh->getID();
  // set highest order bit to ensure id is negative for onlyFinPending streams
  // (i.e. always less than non-onlyFinPending streams) while maintaining stream
  // id priority
  lId |= int64_t(lWh->bufferedSendData_.onlyFinPending()) << 63;
  rId |= int64_t(rWh->bufferedSendData_.onlyFinPending()) << 63;
  return lId < rId;
}

bool WtStreamManager::hasEvent() const noexcept {
  return nextWritable() != nullptr || !ctrlEvents_.empty();
}

uint64_t WtStreamManager::initStreamRecvFc(uint64_t streamId) const noexcept {
  return isBidi(streamId) ? wtConfig_.selfMaxStreamDataBidi
                          : wtConfig_.selfMaxStreamDataUni;
}

uint64_t WtStreamManager::initStreamSendFc(uint64_t streamId) const noexcept {
  return isBidi(streamId) ? wtConfig_.peerMaxStreamDataBidi
                          : wtConfig_.peerMaxStreamDataUni;
}

void WtStreamManager::erase(uint64_t streamId) noexcept {
  bool erased = streams_.erase(streamId) > 0;
  if (!erased) { // may not be in map if invoked via ::shutdown
    return;
  }
  auto& counter = streamsCounter_.getCounter(streamId);
  const uint64_t opened = counter.opened;
  const uint64_t closed = ++counter.closed;
  XCHECK_GE(opened, closed);
  // if peer stream, we may need to advertise additional MaxStreams credit.
  if (isPeer(streamId)) {
    // compute the number of peer openable streams; if it is <= half of the
    // initMaxStreams advertised to peer, we advertise additional MaxStreams
    // credit
    const uint64_t initStreamLimit = isBidi(streamId)
                                         ? wtConfig_.selfMaxStreamsBidi
                                         : wtConfig_.selfMaxStreamsUni;
    auto& maxStreams = maxStreams_.getMaxStreams(streamId);
    const uint64_t openable = maxStreams - closed;
    XLOG(DBG6) << "init=" << initStreamLimit << "; limit=" << maxStreams
               << "; opened=" << opened << "; closed=" << closed;
    if (openable <= initStreamLimit / 2) {
      maxStreams += (initStreamLimit - openable);
      isBidi(streamId) ? enqueueEvent(MaxStreamsBidi{maxStreams})
                       : enqueueEvent(MaxStreamsUni{maxStreams});
    }
  }
}

} // namespace proxygen::coro::detail

/**
 * ReadHandle & WriteHandle implementations here
 */

ReadHandle::ReadHandle(uint64_t id, uint64_t initRecvWnd, Accessor acc) noexcept
    : StreamReadHandle(id),
      smAccessor_(acc),
      streamRecvFc_(initRecvWnd),
      state_(static_cast<HandleState>(acc.isIngress(id))) {
}

folly::SemiFuture<StreamData> ReadHandle::readStreamData() {
  // TODO(@damlaj): hook into interrupt handler, but somehow ensure no UB from
  // concurrent access
  XLOG_IF(FATAL, promise_.valid()) << "one pending read at a time";
  if (!ingress_.chain.empty() || ingress_.fin) {
    auto len = ingress_.chain.computeChainDataLength();
    bytesRead_ += len;
    // only issue conn-level fc if we've rx'd fin
    smAccessor_.maybeGrantFc(ingress_.fin ? nullptr : this, len);
    auto res = StreamData{ingress_.chain.pop(), ingress_.fin};
    XLOG(DBG6) << __func__ << "; len=" << len << "; fin=" << res.fin;
    finish(ingress_.fin);
    return folly::makeSemiFuture(std::move(res));
  }
  // always deliver buffered data (even if rx fin) prior to delivering err
  if (auto ex = ex_) {
    finish(/*done=*/true);
    return folly::makeSemiFuture<StreamData>(std::move(ex));
  }
  auto [p, f] = folly::makePromiseContract<StreamData>();
  promise_ = std::move(p);
  return std::move(f);
}

folly::Expected<folly::Unit, ReadHandle::ErrCode> ReadHandle::stopSending(
    uint32_t error) {
  smAccessor_.stopSending(*this, error);
  // **beware cancel must be last** (`this` can be deleted immediately after)
  cancel(folly::make_exception_wrapper<WtException>(error, "tx stop_sending"));
  return folly::unit;
}

Result ReadHandle::enqueue(StreamData&& data) noexcept {
  XCHECK(!ingress_.fin) << "already rx'd eof";
  auto len = computeChainLength(data.data);
  if (!streamRecvFc_.reserve(len)) {
    return Result::Fail; // error
  }
  if (len > 0) {
    ingress_.chain.appendChain(std::move(data.data));
  }
  ingress_.fin = data.fin;
  XLOG(DBG6) << __func__ << "; len=" << len << "; fin=" << data.fin
             << "; p.valid()=" << promise_.valid();
  if (auto p = resetPromise(); p.valid()) {
    // only issue conn-level fc if we've rx'd fin
    smAccessor_.maybeGrantFc(ingress_.fin ? nullptr : this, len);
    p.setValue(StreamData{ingress_.chain.pop(), ingress_.fin});
    finish(ingress_.fin);
  }
  return Result::Ok;
}

/**
 * Invoked when peer sends rst_stream or when application calls
 * ReadHandle::stopSending – in either case, we must attempt to release
 * connection-level flow control credit. It is unnecessary to
 * release stream-level flow control because the stream's ingress is complete at
 * this point.
 */
void ReadHandle::cancel(folly::exception_wrapper ex,
                        uint64_t reliableSize) noexcept {
  XLOG(DBG8) << __func__ << "; ex=" << ex.what();
  // ensures future reads after reliableSize bytes have been read fail
  ex_ = std::move(ex);
  reliableSize = (reliableSize == kInvalidVarint) ? bytesRead_ : reliableSize;
  if (bytesRead_ < reliableSize) {
    return;
  }
  // any pending reads should be resolved with ex
  if (auto p = resetPromise(); p.valid()) {
    p.setException(ex_);
  }
  // release conn fc credit to peer
  auto len = ingress_.chain.computeChainDataLength();
  smAccessor_.maybeGrantFc(/*rh=*/nullptr, len);
  cs_.requestCancellation();
  finish(/*done=*/true);
}

ReadPromise ReadHandle::resetPromise() noexcept {
  return std::exchange(promise_, emptyReadPromise());
}

void ReadHandle::finish(bool done) noexcept {
  if (done) {
    state_ = Closed;
    smAccessor_.done(*this);
  }
}

WriteHandle::WriteHandle(uint64_t id,
                         uint64_t initSendWnd,
                         Accessor acc) noexcept
    : StreamWriteHandle(id),
      smAccessor_(acc),
      bufferedSendData_(initSendWnd),
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
  bool connBlocked = smAccessor_.connSend().buffer(len);
  bool streamBlocked = bufferedSendData_.enqueue(std::move(data), fin);
  XLOG(DBG6) << __func__ << "; len=" << len << "; fin=" << fin
             << "; connBlocked=" << connBlocked
             << "; streamBlocked=" << streamBlocked;
  if (bufferedSendData_.canSendData()) {
    smAccessor_.onStreamWritable(*this); // stream is now writable
  }
  return streamBlocked ? FcState::BLOCKED : FcState::UNBLOCKED;
}

folly::Expected<folly::Unit, WriteHandle::ErrCode> WriteHandle::resetStream(
    uint32_t error) {
  smAccessor_.resetStream(*this, error, /*reliableSize=*/0);
  // **beware cancel must be last** (`this` can be deleted immediately after)
  cancel(folly::make_exception_wrapper<WtException>(error, "tx reset_stream"));
  return folly::unit;
}

folly::Expected<folly::Unit, WriteHandle::ErrCode> WriteHandle::setPriority(
    uint8_t level, uint32_t order, bool incremental) {
  XLOG(FATAL) << "not implemented";
}

Result WriteHandle::onMaxData(uint64_t offset) {
  if (!bufferedSendData_.grant(offset)) {
    return Result::Fail;
  }
  if (bufferedSendData_.canSendData()) {
    smAccessor_.onStreamWritable(*this); // stream is now writable
  }
  return Result::Ok;
}

folly::Expected<folly::SemiFuture<uint64_t>, WriteHandle::ErrCode>
WriteHandle::awaitWritable() {
  XCHECK(!promise_.valid()) << "at most one pending awaitWritable";
  const auto bufferAvailable = bufferedSendData_.window().getBufferAvailable();
  if (bufferAvailable > 0) {
    return folly::makeSemiFuture(bufferAvailable);
  }
  auto [p, f] = folly::makePromiseContract<uint64_t>();
  promise_ = std::move(p);
  return std::move(f);
}

WritePromise WriteHandle::resetPromise() noexcept {
  return std::exchange(promise_, emptyWritePromise());
}

// TODO(@damlaj): StreamData and DequeueResult should be the same struct
StreamData WriteHandle::dequeue(uint64_t atMost) noexcept {
  XCHECK_NE(state_, Closed) << "dequeue after close";
  auto res = bufferedSendData_.dequeue(atMost);
  const auto bufferAvailable = bufferedSendData_.window().getBufferAvailable();
  if (bufferAvailable > 0) {
    if (auto p = resetPromise(); p.valid()) {
      p.setValue(bufferAvailable);
    }
  }
  if (!bufferedSendData_.canSendData()) {
    smAccessor_.writableStreams().erase(this);
  }
  finish(res.fin);
  return StreamData{std::move(res.data), res.fin};
}

void WriteHandle::cancel(folly::exception_wrapper ex) noexcept {
  XLOG(DBG8) << __func__ << "; ex=" << ex.what();
  ex_ = std::move(ex);
  // any pending awaitWritable should be resolved with ex
  if (auto p = resetPromise(); p.valid()) {
    p.setException(ex_);
  }
  smAccessor_.writableStreams().erase(this);
  cs_.requestCancellation();
  // **beware finish must be last** (`this` can be deleted immediately after)
  finish(/*done=*/true);
}

void WriteHandle::finish(bool done) noexcept {
  if (done) {
    state_ = Closed;
    smAccessor_.done(*this);
  }
}
