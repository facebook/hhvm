/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <glog/logging.h>
#include <limits>
#include <tuple>

namespace proxygen {

template <typename T>
class StateMachine {
 public:
  using State = typename T::State;
  using Event = typename T::Event;

  static State getNewInstance() {
    return T::getInitialState();
  }

  static bool transit(State& state, Event event) {
    bool ok;
    State newState;

    std::tie(newState, ok) = T::find(state, event);
    if (!ok) {
      LOG_EVERY_N(ERROR, 100)
          << T::getName() << ": invalid transition tried: " << state << " "
          << event;
      return false;
    } else {
      VLOG(6) << T::getName() << ": transitioning from " << state << " to "
              << newState;
      state = newState;
      return true;
    }
  }

  static bool canTransit(const State state, Event event) {
    bool ok;

    std::tie(std::ignore, ok) = T::find(state, event);
    return ok;
  }
};

/**
 * The transition table is a [N x M] matrix with N rows and M
 * columns.  Each row represents a state, and each column represents
 * an event.  A valid transition (S1, e) -> S2 is represented by
 * storing the index of S2 (the new state) at transitions[S1, e]. An
 * invalid transition is represented by storing a max value instead of
 * S2 index.
 */
template <class State, class Event>
class TransitionTable {
 private:
  size_t index(State s, Event e) const {
    return static_cast<uint64_t>(s) * nEvents_ + static_cast<uint64_t>(e);
  }

  std::vector<uint8_t> transitions_;
  const size_t nStates_{0};
  const size_t nEvents_{0};

 public:
  TransitionTable(
      size_t nStates,
      size_t nEvents,
      std::vector<std::pair<std::pair<State, Event>, State>> transitions)
      : nStates_(nStates), nEvents_(nEvents) {
    CHECK_LT(static_cast<uint64_t>(nStates),
             std::numeric_limits<uint8_t>::max());
    // Set all transitions to invalid
    transitions_.resize(nStates * nEvents, std::numeric_limits<uint8_t>::max());

    for (auto t : transitions) {
      auto src_state = t.first.first;
      auto event = t.first.second;
      auto dst_state_idx = static_cast<uint8_t>(t.second);
      transitions_[index(src_state, event)] = dst_state_idx;
    }
  }

  TransitionTable() = delete;
  TransitionTable& operator=(const TransitionTable&) = delete;
  TransitionTable(const TransitionTable&) = delete;
  TransitionTable(TransitionTable&& goner)
      : transitions_(std::move(goner.transitions_)),
        nStates_(goner.nStates_),
        nEvents_(goner.nEvents_) {
  }

  std::pair<State, bool> find(State s, Event e) const {
    CHECK_LT(static_cast<uint64_t>(s), nStates_);
    CHECK_LT(static_cast<uint64_t>(e), nEvents_);
    uint8_t result = transitions_[index(s, e)];
    if (result == std::numeric_limits<uint8_t>::max()) {
      return std::make_pair(s, false);
    }
    return std::make_pair(State(result), true);
  }
};

} // namespace proxygen
