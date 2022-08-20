/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iosfwd>
#include <map>
#include <proxygen/lib/utils/StateMachine.h>

namespace proxygen {

class HTTPTransactionIngressSMData {
 public:
  enum class State : uint8_t {
    Start,
    HeadersReceived,
    DatagramReceived,
    RegularBodyReceived,
    ChunkHeaderReceived,
    ChunkBodyReceived,
    ChunkCompleted,
    TrailersReceived,
    UpgradeComplete,
    EOMQueued,
    ReceivingDone,

    // Must be last
    NumStates
  };

  enum class Event : uint8_t {
    // API accessible transitions
    onHeaders,
    onDatagram,
    onBody,
    onChunkHeader,
    onChunkComplete,
    onTrailers,
    onUpgrade,
    onEOM,
    // Internal state transitions
    eomFlushed,

    // Must be last
    NumEvents
  };

  static State getInitialState() {
    return State::Start;
  }

  static std::pair<State, bool> find(State s, Event e);

  static const std::string getName() {
    return "HTTPTransactionIngress";
  }
};

std::ostream& operator<<(std::ostream& os,
                         HTTPTransactionIngressSMData::State s);

std::ostream& operator<<(std::ostream& os,
                         HTTPTransactionIngressSMData::Event e);

using HTTPTransactionIngressSM = StateMachine<HTTPTransactionIngressSMData>;

} // namespace proxygen
