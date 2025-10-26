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

namespace proxygen::coro::detail {

enum WtDir : uint8_t { Client = 0, Server = 1 };

constexpr uint64_t kInvalidVarint = std::numeric_limits<uint64_t>::max();
constexpr uint64_t kMaxVarint = (1ull << 62) - 1;

struct WtStreamManager {
  /**
   * This level-triggered callback (::eventsAvailable) is invoked once control
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
  using Event =
      std::variant<ResetStream, StopSending, MaxConnData, MaxStreamData>;
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
  WtWh* nextWritable() noexcept;

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
