/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/ConnectionTelemetryBridge.h>

#include <atomic>

namespace proxygen {
namespace {

std::atomic<ConnectionTelemetrySink>& sink() {
  static std::atomic<ConnectionTelemetrySink> instance{nullptr};
  return instance;
}

} // namespace

void ConnectionTelemetryBridge::emit(const TraceEvent& event) {
  if (auto registeredSink = sink().load(std::memory_order_acquire)) {
    registeredSink(event);
  }
}

void ConnectionTelemetryBridge::registerSink(
    ConnectionTelemetrySink registeredSink) {
  sink().store(registeredSink, std::memory_order_release);
}

} // namespace proxygen
