// Copyright 2004-present Facebook.  All rights reserved.

#pragma once

#include <iostream>
#include <glog/logging.h>
#include <tuple>

template <typename T>
class StateMachine {
 public:
  typedef typename T::State State;
  typedef typename T::Event Event;

  static State getNewInstance() {
    return T::getInitialState();
  }

  static bool transit(State& state, Event event) {
    bool ok;
    State newState;

    std::tie(newState, ok) = T::find(state, event);
    if (!ok) {
      LOG(ERROR) << "Invalid transition tried: " << state << " " << event;
      return false;
    }
    VLOG(6) << "Transitioning from " << state << " to " << newState;
    state = newState;
    return true;
  }

  static bool canTransit(const State state, Event event) {
    bool ok;

    std::tie(std::ignore, ok) = T::find(state, event);
    return ok;
  }
};
