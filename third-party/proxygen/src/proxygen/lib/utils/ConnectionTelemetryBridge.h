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

struct ConnectionTelemetryBridge {
  static void emit(const TraceEvent& event);

  static void registerSink(ConnectionTelemetrySink sink);

  ConnectionTelemetryBridge() = delete;
};

} // namespace proxygen
