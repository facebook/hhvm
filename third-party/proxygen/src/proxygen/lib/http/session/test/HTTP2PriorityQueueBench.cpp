/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>
#include <folly/Range.h>
#include <proxygen/lib/http/session/HTTP2PriorityQueue.h>

#include <algorithm>

using namespace proxygen;

namespace {
static char* fakeTxn = (char*)0xface0000;

const proxygen::HTTPCodec::StreamID kRootNodeId =
    std::numeric_limits<uint64_t>::max();

proxygen::HTTPTransaction* makeFakeTxn(proxygen::HTTPCodec::StreamID id) {
  return (proxygen::HTTPTransaction*)(fakeTxn + id);
}
} // namespace

BENCHMARK(Encode, iters) {
  HTTP2PriorityQueue q_(WheelTimerInstance(), kRootNodeId);
  uint64_t depth = 0;
  for (size_t i = 0; i < iters; ++i) {
    HTTPCodec::StreamID id = i * 2 + 1;
    q_.addTransaction(
        id, http2::DefaultPriority, makeFakeTxn(id), false, &depth);
  }
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
