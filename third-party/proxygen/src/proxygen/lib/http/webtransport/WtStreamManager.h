/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <variant>

#include <folly/container/F14Set.h>
#include <proxygen/lib/http/webtransport/StreamPriorityQueue.h>

/**
 * get rid of WebTransport.h dependency, extract StreamReadHandle &
 * StreamWriteHandle into a different TU?
 */
#include <proxygen/lib/http/webtransport/WebTransport.h>
#include <proxygen/lib/http/webtransport/WtEgressContainer.h>

/**
 * This comment block documents the api of WtStreamManager (i.e. its behaviour).
 * Please refer to the documentation in WtStreamManager.cpp for the
 * implementation details (i.e. the how).
 *
 * WtStreamManager, as the name suggestions, simply manages the stream state of
 * WebTransport streams. Following good design philosophy, as much as possible
 * is hidden from the user of this class.
 *
 * There are two channels to interact with this class:
 *
 * – The application via the handles (StreamWriteHandle & StreamReadHandle) and
 *   their respective methods (e.g. WriteHandle::writeStreamData,
 *   ReadHandle::stopSending, etc.).
 *
 * – The backing transport (http/2, quic, etc.) via the methods in this class
 *   (e.g. onMaxStreams, onMaxData, ::enqueue(ReadHandle&),
 *   ::dequeue(WriteHandle&), etc.)
 *
 * There are two channels to communicate events to the backing transport, via
 * IngressCallback and EgressCallback.
 *
 *  – IngressCallback lets the backing transport know that a new peer stream is
 *    available to read/write. This is a simple uint64_t, and the backing
 *    transport can invoke ::getOrCreateBidiHandle to inspect whether this is a
 *    bidi or uni stream
 *
 *  – EgressCallback lets the backing transport know that there is an event
 *    available for the write loop to action on – there are two main
 *    EgressCallback events:
 *
 *    – Control frames that need to be serialized/sent by the transport (e.g.
 *      reset_stream, stop_sending, etc.). The transport should invoke
 *      ::moveEvents and install a visitor to ensure all control-frames are
 *      handled appropriately.
 *
 *    – A stream that is now writable. The transport should peek the priority
 *      queue and dequeue data from streams by ID.
 *
 * A note about stream handles – everything is dervied from WtDir (the role of
 * the endpoint, e.g. client or server) and stream id. Any attempt to
 * access/create a handle for an invalid direction will return nullptr. For
 * example, a client invoking ::getEgressHandle for a stream id that is
 * logically a server-initiated unidirectional stream (e.g. id=0x03) will return
 * nullptr.
 */

namespace proxygen::detail {

// value maps to lsb of stream id as per rfc9000
enum WtDir : uint8_t { Client = 0, Server = 1 };

constexpr uint64_t kInvalidVarint = std::numeric_limits<uint64_t>::max();
constexpr uint64_t kMaxVarint = (1ull << 62) - 1;

struct WtStreamManager {
  /**
   * This edge-triggered callback (::eventsAvailable) is invoked once control
   * events are available to be dequeued by the backing transport – (e.g.
   * connection- and stream-level flow control, reset_stream, stop_sending,
   * etc.)
   *
   * It is also invoked once there is a writable stream (i.e. the priority
   * queue becomes non-empty)
   */
  struct EgressCallback {
    virtual ~EgressCallback() = default;
    virtual void eventsAvailable() noexcept = 0;
  };
  /**
   * IngressCallback::onNewPeerStream is invoked whenever a new peer stream has
   * been allocated. As of now, the backing transport must wait an EventBase
   * loop before passing the handle to the application (i.e.
   * WebTransportHandler).
   */
  struct IngressCallback {
    virtual ~IngressCallback() = default;
    virtual void onNewPeerStream(uint64_t streamId) noexcept = 0;
  };

  struct WtConfig {
    static constexpr auto kDefaultFc = std::numeric_limits<uint16_t>::max();
    // values we've advertised to the peer
    uint64_t selfMaxStreamsBidi{1};
    uint64_t selfMaxStreamsUni{1};
    uint64_t selfMaxConnData{kDefaultFc};
    uint64_t selfMaxStreamDataBidi{kDefaultFc};
    uint64_t selfMaxStreamDataUni{kDefaultFc};

    // values peer has advertised to us
    uint64_t peerMaxStreamsBidi{1};
    uint64_t peerMaxStreamsUni{1};
    uint64_t peerMaxConnData{kDefaultFc};
    uint64_t peerMaxStreamDataBidi{kDefaultFc};
    uint64_t peerMaxStreamDataUni{kDefaultFc};
  };

  WtStreamManager(WtDir dir,
                  const WtConfig& config,
                  EgressCallback& egressCb,
                  IngressCallback& ingressCb,
                  quic::PriorityQueue& priorityQueue) noexcept;
  ~WtStreamManager() noexcept;

  using WtException = WebTransport::Exception;
  using WtWriteHandle = WebTransport::StreamWriteHandle;
  using WtReadHandle = WebTransport::StreamReadHandle;
  using StreamData = WebTransport::StreamData;
  using ReadPromise = folly::Promise<StreamData>;
  using ReadFut = folly::SemiFuture<StreamData>;
  /**
   * - Gets the stream if it exists
   * - Otherwise attempts to create the stream if sufficient stream credit
   *   exists & not shutdown
   * - Otherwise returns nullptr
   */
  WtWriteHandle* getOrCreateEgressHandle(uint64_t streamId) noexcept;
  WtReadHandle* getOrCreateIngressHandle(uint64_t streamId) noexcept;
  WebTransport::BidiStreamHandle getOrCreateBidiHandle(
      uint64_t streamId) noexcept;

  // Gets a stream if it already exists, otherwise nullptr
  [[nodiscard]] WebTransport::BidiStreamHandle getBidiHandle(
      uint64_t streamId) const noexcept;
  [[nodiscard]] WtWriteHandle* getEgressHandle(
      uint64_t streamId) const noexcept;
  [[nodiscard]] WtReadHandle* getIngressHandle(
      uint64_t streamId) const noexcept;

  /**
   * Initiators of streams should use this api, attempts to create the next
   * consecutive egress/bidi handle. This is not applicable to quic as quic
   * stream ids may not be consecutive w.r.t WebTransport streams. Usage over
   * quic should first successfully create the quic stream and use the above
   * getOrCreate(Ingress|Egress)Handle apis to create a handle.
   */
  WtWriteHandle* createEgressHandle() noexcept;
  WebTransport::BidiStreamHandle createBidiHandle() noexcept;

  /**
   * per-stream callbacks that may be useful to transports that support stream
   * multiplexing (e.g. quic)
   */
  struct ReadCallback {
    virtual ~ReadCallback() noexcept = default;
    // invoked when the read handle has available buffer space
    virtual void readReady(WtReadHandle&) noexcept = 0;
  };
  void setReadCb(WtReadHandle& rh, ReadCallback* rcb) noexcept;
  // ignores wt connection-level flow control (this should be set to kMaxVarint
  // in quic)
  bool hasPendingData(const WtWriteHandle& wh) const noexcept;
  uint64_t bufferedBytes(const WtReadHandle& rh) const noexcept;

  enum Result : uint8_t { Fail = 0, Ok = 1 };
  /**
   * invoke when receiving max_streams frame from peer – returns Ok if
   * successful (monotonically increasing max_streams), Fail otherwise
   */
  struct MaxStreams {
    uint64_t maxStreams{0};
  };
  struct MaxStreamsBidi : MaxStreams {};
  struct MaxStreamsUni : MaxStreams {};
  Result onMaxStreams(MaxStreamsBidi);
  Result onMaxStreams(MaxStreamsUni);

  /**
   * invoke when receiving wt_max_data & wt_max_stream_data (latter in http/2
   * only) – returns Ok if successful (i.e. monotonically increasing max_data),
   * Fail otherwise
   */
  struct MaxConnData {
    uint64_t maxData{0};
  };
  struct MaxStreamData : MaxConnData {
    uint64_t streamId{0};
  };
  Result onMaxData(MaxConnData) noexcept;
  Result onMaxData(MaxStreamData) noexcept;

  /**
   * invoke when receiving a stop_sending – returns bool if stream was found
   * TODO(@damlaj): enforce sending a rst_stream in response to stop_sending
   */
  struct StopSending {
    uint64_t streamId{0};
    uint64_t err{0};
  };
  Result onStopSending(StopSending) noexcept;

  struct ResetStream {
    uint64_t streamId{0};
    uint64_t err{0};
    uint64_t reliableSize{0};
  };
  /**
   * Invoke when receiving a rst_stream capsule – returns Ok if stream was
   * found, Fail otherwise
   *
   * TODO(@damlaj): implement reliable_offset
   */
  Result onResetStream(ResetStream) noexcept;

  /**
   * Invoke when receiving a drain_session capsule. ::onDrainSession has no
   * immediate side-effect, all streams are untouched. It does however prevent
   * the creation of any new streams (both self- and peer-initiated)
   */
  struct DrainSession {};
  void onDrainSession(DrainSession) noexcept;

  /**
   * Invoke when receiving a close_session capsule. ::onCloseSession has the
   * immediate side-effect of deallocating every stream. In addition, it
   * prevents the creation of any new streams (both self- and peer-initiated).
   */
  struct CloseSession {
    uint64_t err{0};
    std::string msg;
  };
  void onCloseSession(CloseSession) noexcept;

  /**
   * Enqueues data into read handle – returns Fail if recv window overflowed
   * (either conn or stream) or Ok otherwise. Receive window is considered
   * "strict", and any overflow should be treated as an error.
   */
  Result enqueue(WtReadHandle&, StreamData data) noexcept;

  /**
   * Dequeues buffered data from the WtWriteHandle; if there is no data to be
   * dequeued, it returns StreamData{.data=nullptr, .fin=false}
   */
  StreamData dequeue(WtWriteHandle&, uint64_t atMost) noexcept;

  /**
   * Events are communicated to the backing transport (http/2 or http/3) via
   * Callback::eventsAvailable – the user can subsequently dequeue all events
   * using the below ::moveEvents(). All these events are strictly control
   * frames by design, as they are not flow controlled.
   */
  using Event = std::variant<ResetStream,
                             StopSending,
                             MaxConnData,
                             MaxStreamData,
                             MaxStreamsBidi,
                             MaxStreamsUni,
                             DrainSession,
                             CloseSession>;
  std::vector<Event> moveEvents() noexcept {
    return std::move(ctrlEvents_);
  }

  struct Accessor; // used by Read&Write handle to access private members of
                   // this class
  friend struct Accessor;

  /**
   * DEPRECATED: Use the priority queue directly instead.
   *
   * Returns the next writable stream if the head of the priority queue is a
   * stream. Returns nullptr if:
   * - The queue is empty
   * - The head of the queue is not a stream (e.g., a datagram)
   */
  [[nodiscard]] WtWriteHandle* nextWritable() const noexcept;

  [[nodiscard]] bool hasStreams() const noexcept {
    return !streams_.empty();
  }

  [[nodiscard]] bool isClosed() const noexcept {
    return shutdown_ && !hasStreams();
  }

  /**
   * Locally initiated drain of the WtStreamManager. This will enqueue an event
   * to send to the peer (of type DrainSession, notified via Callback). Similar
   * to ::onDrainSession, this will leave existing streams untouched. However,
   * no new streams can be created following this invocation.
   */
  void drain() noexcept;

  /**
   * Locally initiated shutdown of the WtStreamManager. This will enqueue an
   * event to send to the peer (of type CloseSession, notified via Callback).
   * Similar to ::onCloseSession, this has an immediate side-effect of
   * deallocating every stream and prevents the creation of new streams.
   */
  void shutdown(CloseSession) noexcept;

  // returns true iff we can create self-initiated uni/bidi streams
  [[nodiscard]] bool canCreateUni() const noexcept;
  [[nodiscard]] bool canCreateBidi() const noexcept;

 private:
  [[nodiscard]] bool isSelf(uint64_t streamId) const;
  [[nodiscard]] bool isPeer(uint64_t streamId) const;
  [[nodiscard]] bool isEgress(uint64_t streamId) const;
  [[nodiscard]] bool isIngress(uint64_t streamId) const;
  [[nodiscard]] bool isUni(uint64_t streamId) const;
  [[nodiscard]] bool isBidi(uint64_t streamId) const;

  void enqueueEvent(Event&& ev) noexcept;
  void onStreamWritable(WtWriteHandle& wh) noexcept;
  [[nodiscard]] bool hasEvent() const noexcept;
  struct BidiHandle;
  friend struct BidiHandle;
  BidiHandle* getOrCreateBidiHandleImpl(uint64_t streamId) noexcept;
  [[nodiscard]] uint64_t initStreamRecvFc(uint64_t streamId) const noexcept;
  [[nodiscard]] uint64_t initStreamSendFc(uint64_t streamId) const noexcept;
  void erase(uint64_t streamId) noexcept;

  // as per rfc-9000
  enum StreamType : uint8_t {
    ClientBidi = 0x00,
    ServerBidi = 0x01,
    ClientUni = 0x02,
    ServerUni = 0x03,
    Max = 0x04
  };
  // value maps to second lsb as per rfc-9000
  enum WtStreamDir : uint8_t { Bidi = 0x00, Uni = 0x01 };
  [[nodiscard]] bool streamLimitExceeded(uint64_t streamId) const noexcept;

  const WtDir dir_;

  std::map<uint64_t, std::unique_ptr<BidiHandle>> streams_;

  // used for stream initiators
  struct NextStreamIds {
    uint64_t bidi{};
    uint64_t uni{};
  } nextStreamIds_;

  // index into arrays using the lower two bits of stream id
  struct StreamsCounter {
    uint64_t opened{0};
    uint64_t closed{0};
  };

  struct StreamsCounterContainer {
    using Type = std::array<StreamsCounter, StreamType::Max>;
    explicit StreamsCounterContainer() noexcept = default;
    StreamsCounter& getCounter(uint64_t streamId) noexcept;
    [[nodiscard]] const StreamsCounter& getCounter(
        uint64_t streamId) const noexcept;

   private:
    Type streamsCounter_;
  } streamsCounter_;

  struct MaxStreamsContainer {
    using Type = std::array<uint64_t, StreamType::Max>;
    explicit MaxStreamsContainer(Type maxStreams) noexcept;
    uint64_t& getMaxStreams(uint64_t streamId) noexcept;
    [[nodiscard]] uint64_t getMaxStreams(uint64_t streamId) const noexcept;

   private:
    Type maxStreams_;
  } maxStreams_;

  /**
   * Priority queue wrapper for scheduling stream egress.
   * Streams are inserted when they have buffered data & stream-level FC.
   * Removed when out of data or stream FC.
   * Connection-FC-blocked streams are tracked separately and re-inserted on
   * onMaxData.
   */
  StreamPriorityQueue writableStreams_;

  /**
   * Streams that have data + stream FC but are blocked by connection FC.
   * These are re-inserted into writableStreams_ when connection FC is granted.
   */
  folly::F14FastSet<WtWriteHandle*> connFcBlockedStreams_;

  WtConfig wtConfig_;
  uint64_t connBytesRead_{0};
  FlowController connRecvFc_;
  BufferedFlowController connSendFc_;
  EgressCallback& egressCb_;
  IngressCallback& ingressCb_;
  std::vector<Event> ctrlEvents_;
  bool drain_ : 1 {false};
  bool shutdown_ : 1 {false};

  // helper functions to compute next stream ids and max streams
  static NextStreamIds nextStreamIds(WtDir) noexcept;
  static MaxStreamsContainer::Type maxStreams(WtDir, const WtConfig&) noexcept;
  static StreamType streamType(uint64_t streamId) noexcept;
};

} // namespace proxygen::detail
