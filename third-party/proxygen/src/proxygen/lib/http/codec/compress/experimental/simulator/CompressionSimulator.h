/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/compress/experimental/simulator/CompressionScheme.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/CompressionTypes.h>

#include <chrono>
#include <folly/Random.h>
#include <folly/io/async/EventBase.h>
#include <unordered_map>
#include <vector>

namespace proxygen { namespace compress {

class CompressionSimulator {
 public:
  explicit CompressionSimulator(SimParams p) : params_(p) {
  }

  bool readInputFromFileAndSchedule(const std::string& filename);
  void run();

  // Called from CompressionScheme::runLoopCallback
  void flushSchemePackets(CompressionScheme* scheme);
  void flushPacket(CompressionScheme* scheme);

 private:
  void flushRequests(CompressionScheme* scheme);
  void setupRequest(uint16_t seqn,
                    HTTPMessage&& msg,
                    std::chrono::milliseconds encodeDelay);
  CompressionScheme* getScheme(folly::StringPiece host);
  std::unique_ptr<CompressionScheme> makeScheme();
  std::pair<FrameFlags, std::unique_ptr<folly::IOBuf>> encode(
      CompressionScheme* scheme, bool newPacket, uint16_t seqn);
  void decodePacket(CompressionScheme* scheme,
                    std::list<CompressionScheme::BlockInfo>& blocks);
  void decode(CompressionScheme* scheme,
              FrameFlags flags,
              std::unique_ptr<folly::IOBuf> encodedReq,
              SimStreamingCallback& cb);
  void scheduleEvent(folly::Function<void()> f, std::chrono::milliseconds ms);
  void sendAck(CompressionScheme* scheme,
               std::unique_ptr<CompressionScheme::Ack> ack);
  void recvAck(CompressionScheme* scheme,
               std::unique_ptr<CompressionScheme::Ack> ack);

  std::chrono::milliseconds deliveryDelay();
  std::chrono::milliseconds rtt();
  std::chrono::milliseconds one_half_rtt();
  std::chrono::milliseconds rxmitDelay();
  bool loss();
  bool delayed();
  std::chrono::milliseconds extraDelay();
  uint32_t minOOOThresh();

  SimParams params_;
  std::vector<proxygen::HTTPMessage> requests_;
  folly::EventBase eventBase_;
  // Map of domain-name to compression scheme
  std::unordered_map<std::string, std::unique_ptr<CompressionScheme>> domains_;
  std::vector<SimStreamingCallback> callbacks_;
  folly::Random::DefaultGenerator rng_{
      static_cast<folly::Random::DefaultGenerator::result_type>(params_.seed)};
  SimStats stats_;
};
}} // namespace proxygen::compress
