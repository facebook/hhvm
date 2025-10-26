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
#include <set>
#include <variant>

/**
 * get rid of WebTransport.h dependency, extract StreamReadHandle &
 * StreamWriteHandle into a different TU?
 */
#include <proxygen/lib/http/coro/util/WtEgressContainer.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>

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
 * As of now, there's a single channel to communicate events to the backing
 * transport –via WtStreamManager::Callback. This simply lets the backing
 * transport know that there is an event available for the write loop to action
 * on – there are two main events:
 *
 * – Control frames that need to be serialized/sent by the transport (e.g.
 *   reset_stream, stop_sending, etc.). The transport should invoke ::moveEvents
 *   and install a visitor to ensure all control-frames are handled
 *   appropriately.
 *
 * – A stream that is now writable (e.g. ::nextWritable will return
 *   non-nullptr). The transport should query the nextWritable stream and
 *   dequeue data from the handle.
 *
 *
 * A note about stream handles – everything is dervied from WtDir (the role of
 * the endpoint, e.g. client or server) and stream id. Any attempt to
 * access/create a handle for an invalid direction will return nullptr. For
 * example, a client invoking ::getEgressHandle for a stream id that is
 * logically a server-initiated unidirectional stream (e.g. id=0x03) will return
 * nullptr.
 */

namespace proxygen::coro::detail {

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
   * It is also invoked once there is a writable stream (i.e. nextWritable()
   * transitions from returning nullptr to returning a valid egress handle)
   */
  struct Callback {
    virtual ~Callback() = default;
    virtual void eventsAvailable() noexcept = 0;
  };

  struct WtMaxStreams {
    uint64_t bidi{0};
    uint64_t uni{0};
  };

  WtStreamManager(WtDir dir,
                  WtMaxStreams self,
                  WtMaxStreams peer,
                  Callback& cb) noexcept;
  ~WtStreamManager() noexcept;

  using WtException = WebTransport::Exception;
  using WtWriteHandle = WebTransport::StreamWriteHandle;
  using WtReadHandle = WebTransport::StreamReadHandle;
  using StreamData = WebTransport::StreamData;
  using ReadPromise = folly::Promise<StreamData>;
  using ReadFut = folly::SemiFuture<StreamData>;
  /**
   * a bit of an odd api, but:
   *  - Gets the stream if already present
   *  - Otherwise if equals next expected stream, creates it (ordered delivery
   *  - property of wt over http/2, http/3 requires a mapping of transport id ->
   *    logical id)
   *  - Otherwise nullptr
   */
  WtWriteHandle* getOrCreateEgressHandle(uint64_t streamId) noexcept;
  WtReadHandle* getOrCreateIngressHandle(uint64_t streamId) noexcept;
  WebTransport::BidiStreamHandle getOrCreateBidiHandle(
      uint64_t streamId) noexcept;

  /**
   * initiators of streams should use this api, attempts to create the next
   * consecutive egress/bidi handle
   */
  WtWriteHandle* createEgressHandle() noexcept;
  WebTransport::BidiStreamHandle createBidiHandle() noexcept;

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
    /*uint64_t reliable_offset;*/
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
                             CloseSession>;
  std::vector<Event> moveEvents() noexcept {
    return std::move(ctrlEvents_);
  }

  struct Accessor; // used by Read&Write handle to access private members of
                   // this class
  friend struct Accessor;

  /**
   * nextWritable() returns the next writable stream **if exists and there is
   * available connection flow control**. Otherwise, returns nullptr
   */
  WtWriteHandle* nextWritable() const noexcept;

  bool hasStreams() const noexcept {
    return !streams_.empty();
  }

  bool isClosed() const noexcept {
    return drain_ && !hasStreams();
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

 private:
  [[nodiscard]] bool isSelf(uint64_t streamId) const;
  [[nodiscard]] bool isPeer(uint64_t streamId) const;
  [[nodiscard]] bool isEgress(uint64_t streamId) const;
  [[nodiscard]] bool isIngress(uint64_t streamId) const;
  [[nodiscard]] bool isUni(uint64_t streamId) const;
  [[nodiscard]] bool isBidi(uint64_t streamId) const;
  uint64_t* nextExpectedStream(uint64_t streamId);
  void enqueueEvent(Event&& ev) noexcept;
  void onStreamWritable(WtWriteHandle& wh) noexcept;
  void shutdownImpl(uint32_t, std::string) noexcept;
  bool hasEvent() const noexcept;

  WtDir dir_;
  struct NextStreams {
    uint64_t bidi{0};
    uint64_t uni{0};  // expected consecutive stream ids
    WtMaxStreams max; // max concurrency
  };
  NextStreams selfNextStreams_;
  NextStreams peerNextStreams_;

  struct BidiHandle;
  std::map<uint64_t, std::unique_ptr<BidiHandle>> streams_;

  // writable streams ordered by stream id
  // TODO(@damlaj): support priorities
  struct Compare {
    bool operator()(const WtWriteHandle* l, const WtWriteHandle* r) const;
  };
  /**
   * Any stream with buffered data & available stream-level flow control is in
   * this map, regardless of available connection-level flow control. This is so
   * an ::onMaxData does not have to iterate over all streams to determine which
   * streams are writable.
   *
   * ::nextWritable, however, will account for the connection-level flow control
   */
  std::set<WtWriteHandle*, Compare> writableStreams_;

  FlowController connRecvFc_;
  BufferedFlowController connSendFc_;
  Callback& cb_;
  std::vector<Event> ctrlEvents_;
  bool drain_{false};

  // helper functions to compute next streams
  static NextStreams selfNextStreams(WtDir, WtMaxStreams);
  static NextStreams peerNextStreams(WtDir, WtMaxStreams);
};

} // namespace proxygen::coro::detail
