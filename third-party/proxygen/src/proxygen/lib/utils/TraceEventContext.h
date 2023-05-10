/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <vector>

namespace proxygen {

struct TraceEventObserver;
class TraceEvent;

class TraceEventContext {
 public:
  // Optional parent id for all sub trace events to add.
  uint32_t parentID;

  TraceEventContext(uint32_t pID,
                    std::vector<TraceEventObserver*> observers,
                    bool allTraceEventNeeded = false)
      : parentID(pID),
        observers_(std::move(observers)),
        allTraceEventNeeded_(allTraceEventNeeded) {
  }

  explicit TraceEventContext(uint32_t pID = 0,
                             TraceEventObserver* observer = nullptr,
                             bool allTraceEventNeeded = false)
      : parentID(pID), allTraceEventNeeded_(allTraceEventNeeded) {
    if (observer) {
      observers_.push_back(observer);
    }
  }

  void traceEventAvailable(const TraceEvent& event);

  bool isAllTraceEventNeeded() const;

 private:
  // Observer vector to observe all trace events about to occur
  std::vector<TraceEventObserver*> observers_;

  // Whether the observers actually care about all trace events from this
  // context or only necessary ones.
  bool allTraceEventNeeded_;
};

} // namespace proxygen
