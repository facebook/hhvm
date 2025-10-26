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

/**
 * get rid of WebTransport.h dependency, extract StreamReadHandle &
 * StreamWriteHandle into a different TU?
 */
#include <proxygen/lib/http/webtransport/FlowController.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>

namespace proxygen::coro::detail {

enum WtDir : uint8_t { Client = 0, Server = 1 };

constexpr uint64_t kInvalidVarint = std::numeric_limits<uint64_t>::max();
constexpr uint64_t kMaxVarint = (1ull << 62) - 1;

struct WtStreamManager {
  struct WtMaxStreams {
    uint64_t bidi{0};
    uint64_t uni{0};
  };

  WtStreamManager(WtDir dir, WtMaxStreams self, WtMaxStreams peer);
  ~WtStreamManager();

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
   * Enqueues data into read handle – returns bool indicating if recv window
   * overflowed (either conn or stream)
   */
  bool enqueue(WtRh&, StreamData data) noexcept;

 private:
  bool isSelf(uint64_t streamId) const;
  bool isPeer(uint64_t streamId) const;
  bool isEgress(uint64_t streamId) const;
  bool isIngress(uint64_t streamId) const;
  bool isUni(uint64_t streamId) const;
  bool isBidi(uint64_t streamId) const;
  uint64_t* nextExpectedStream(uint64_t streamId);

  WtDir dir_;
  struct NextStreams {
    uint64_t bidi{0}, uni{0}; // expected consecutive stream ids
    WtMaxStreams max;         // max concurrency
  } self_, peer_;

  struct BidiHandle;
  std::map<uint64_t, std::unique_ptr<BidiHandle>> streams_;

  FlowController recv_;

  // helper functions to compute next streams
  static NextStreams selfNextStreams(WtDir, WtMaxStreams);
  static NextStreams peerNextStreams(WtDir, WtMaxStreams);
};

} // namespace proxygen::coro::detail
