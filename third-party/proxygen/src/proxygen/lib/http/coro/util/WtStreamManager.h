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
                  Callback& cb);
  ~WtStreamManager();

  using WtException = WebTransport::Exception;
  using WtWh = WebTransport::StreamWriteHandle;
  using WtRh = WebTransport::StreamReadHandle;
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
  WtWh* getEgressHandle(uint64_t streamId);
  WtRh* getIngressHandle(uint64_t streamId);
  WebTransport::BidiStreamHandle getBidiHandle(uint64_t streamId);

  /**
   * initiators of streams should use this api, attempts to create the next
   * consecutive egress/bidi handle
   */
  WtWh* nextEgressHandle() noexcept;
  WebTransport::BidiStreamHandle nextBidiHandle() noexcept;

  /**
   * invoke when receiving max_streams frame from peer – returns bool if
   * successful (i.e. valid incremental max_streams)
   */
  struct MaxStreams {
    uint64_t maxStreams{0};
  };
  struct MaxStreamsBidi : MaxStreams {};
  struct MaxStreamsUni : MaxStreams {};
  bool onMaxStreams(MaxStreamsBidi);
  bool onMaxStreams(MaxStreamsUni);

  /**
   * invoke when receiving wt_max_data & wt_max_stream_data (latter in http/2
   * only) – returns bool if successful (i.e. valid incremental max_data)
   */
  struct MaxConnData {
    uint64_t maxData{0};
  };
  struct MaxStreamData : MaxConnData {
    uint64_t streamId{0};
  };
  bool onMaxData(MaxConnData) noexcept;
  bool onMaxData(MaxStreamData) noexcept;

  /**
   * invoke when receiving a stop_sending – returns bool if stream was found
   * TODO(@damlaj): enforce sending a rst_stream in response to stop_sending
   */
  struct StopSending {
    uint64_t streamId{0};
    uint64_t err{0};
  };
  bool onStopSending(StopSending) noexcept;

  struct ResetStream {
    uint64_t streamId{0};
    uint64_t err{0};
    /*uint64_t reliable_offset;*/
  };
  /**
   * invoke when receiving a rst_stream – returns bool if stream was found
   * TODO(@damlaj): implement reliable_offset
   */
  bool onResetStream(ResetStream) noexcept;

  /**
   * invoke when receiving a drain_session capsule
   */
  struct DrainSession {};
  void onDrainSession(DrainSession) noexcept;

  /**
   * invoke when receiving a close_session capsule
   */
  struct CloseSession {
    uint64_t err{0};
    std::string msg;
  };
  void onCloseSession(CloseSession) noexcept;

  /**
   * Enqueues data into read handle – returns bool indicating if recv window
   * overflowed (either conn or stream)
   */
  bool enqueue(WtRh&, StreamData data) noexcept;

  /**
   * Dequeues data from write handle
   * - invariant, must be the stream received from ::nextWritable
   * -  TODO(@damlaj): combine into a single function?
   */
  StreamData dequeue(WtWh&, uint64_t atMost) noexcept;

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
    return std::move(events_);
  }

  struct Accessor; // used by Read&Write handle to access private members of
                   // this class
  friend struct Accessor;

  /**
   * nextWritable() returns the next writable stream **if exists and there is
   * available connection flow control**. Otherwise, returns nullptr
   */
  WtWh* nextWritable() const noexcept;

  bool hasStreams() const noexcept {
    return !streams_.empty();
  }

  bool isClosed() const noexcept {
    return drain_ && !hasStreams();
  }

  // locally initiated drain
  void drain() noexcept;

  // locally initiated shutdown
  void shutdown(CloseSession) noexcept;

 private:
  bool isSelf(uint64_t streamId) const;
  bool isPeer(uint64_t streamId) const;
  bool isEgress(uint64_t streamId) const;
  bool isIngress(uint64_t streamId) const;
  bool isUni(uint64_t streamId) const;
  bool isBidi(uint64_t streamId) const;
  uint64_t* nextExpectedStream(uint64_t streamId);
  void enqueueEvent(Event&& ev) noexcept;
  void onStreamWritable(WtWh& wh) noexcept;
  void shutdownImpl(uint32_t, std::string) noexcept;
  bool hasEvent() const noexcept;

  WtDir dir_;
  struct NextStreams {
    uint64_t bidi{0}, uni{0}; // expected consecutive stream ids
    WtMaxStreams max;         // max concurrency
  } self_, peer_;

  struct BidiHandle;
  std::map<uint64_t, std::unique_ptr<BidiHandle>> streams_;

  // writable streams ordered by stream id
  // TODO(@damlaj): support priorities
  struct Compare {
    bool operator()(const WtWh* l, const WtWh* r) const;
  };
  // streams are in this map regardless of available connection flow control
  std::set<WtWh*, Compare> writableStreams_;

  FlowController recv_;
  BufferedFlowController send_;
  Callback& cb_;
  std::vector<Event> events_;
  bool drain_{false};

  // helper functions to compute next streams
  static NextStreams selfNextStreams(WtDir, WtMaxStreams);
  static NextStreams peerNextStreams(WtDir, WtMaxStreams);
};

} // namespace proxygen::coro::detail
