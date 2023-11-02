/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBufQueue.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>
#include <vector>

namespace devious {

enum class BatonSessionError {
  DA_YAMN = 0x01,   //	Insufficient stream credit to continue the protocol
  BRUH = 0x02,      //	Received a malformed Baton message
  GAME_OVER = 0x03, //	All baton streams have been reset
  BORED = 0x04,     //	Got tired of waiting for the next message
  SUS = 0x05,       //	Received an unexpected Baton message
};

enum class BatonStreamError {
  IDC = 0x01,      // I don't care about this stream
  WHATEVER = 0x02, // The peer asked for this
  I_LIED = 0x03,   // Spontaneous reset
};

class DeviousBaton {
 public:
  enum class Mode { CLIENT, SERVER };
  using StartReadFn =
      std::function<void(proxygen::WebTransport::StreamReadHandle*)>;
  DeviousBaton(proxygen::WebTransport* inWt, Mode mode, StartReadFn startReadFn)
      : wt_(inWt), mode_(mode), startReadFn_(startReadFn) {
  }

  folly::Expected<folly::Unit, uint16_t> onRequest(
      const proxygen::HTTPMessage& request);

  void start();

  proxygen::HTTPMessage makeRequest(uint64_t version,
                                    uint64_t count,
                                    std::vector<uint8_t> batons);

  struct BatonMessageState {
    enum State { PAD_LEN, PAD, BATON, DONE } state{PAD_LEN};
    folly::IOBufQueue bufQueue;
    uint64_t paddingRemaining;
    uint8_t baton;
  };

  void onStreamData(uint64_t streamId,
                    BatonMessageState& state,
                    std::unique_ptr<folly::IOBuf> data,
                    bool fin);

  folly::Expected<folly::Unit, BatonSessionError> onBatonMessageData(
      BatonMessageState& state, std::unique_ptr<folly::IOBuf> data, bool fin);

  enum WhoFinished { SELF, PEER, NO_ONE };

  void onBatonFinished(WhoFinished who, bool reset);

  enum MessageSource { UNI, PEER_BIDI, SELF_BIDI };

  folly::Expected<folly::Unit, proxygen::WebTransport::ErrorCode> closeSession(
      folly::Optional<uint32_t> error) {
    return wt_->closeSession(std::move(error));
  }

 private:
  proxygen::WebTransport* wt_{nullptr};
  Mode mode_;
  uint64_t activeBatons_{0};
  uint64_t resetBatons_{0};
  uint64_t finishedBatons_{0};
  std::vector<uint8_t> batons_;
  StartReadFn startReadFn_;

  folly::Expected<WhoFinished, BatonSessionError> onBatonMessage(
      uint64_t inStreamId, MessageSource arrivedOn, uint8_t baton);
};

} // namespace devious
