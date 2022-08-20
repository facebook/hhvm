/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/utils/TraceEvent.h>

namespace proxygen {

/*
 * Observer interface to record trace events.
 *
 * Subclasses of TraceEventObserver may log the trace events
 * to a destination analytics pipeline or forward them elsewhere.
 */
struct TraceEventObserver {
  virtual ~TraceEventObserver() {
  }
  /**
   * Lets the handler receive an arbitrary TraceEvent.
   */
  virtual void traceEventAvailable(TraceEvent) noexcept {
  }
  virtual void emitTraceEvents(std::vector<TraceEvent>) noexcept {
  }
};

} // namespace proxygen
