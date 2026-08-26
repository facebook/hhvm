/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/ThreadLocal.h>
#include <folly/lang/Align.h>

#include <thrift/lib/cpp2/fast_thrift/common/Stats.h>

namespace apache::thrift::fast_thrift::connection {

/**
 * Per-EventBase block of the connection layer's counters.
 *
 * Deliberately separate from ServerStatsShard rather than folded into it.
 * That shard is the thrift/rocket message counters, and its shape is pinned
 * by FastThriftStatsConcept — a contract the *client* metrics handlers also
 * satisfy, and one embedders are told to implement themselves. Connection
 * lifecycle is neither a message count nor a client-side concern, so putting
 * it there would force every client stats type, and every embedder's, to
 * carry counters that mean nothing to them.
 *
 * Aligned so that two shards never share a cache line, for the same reason
 * ServerStatsShard is: shards are separately heap-allocated and could
 * otherwise land adjacent, reintroducing the cross-core interference the
 * single-writer design exists to avoid.
 */
struct alignas(folly::hardware_destructive_interference_size)
    ConnectionStatsShard {
  // Counted once a connection is established — after the handshake, which is
  // where the classic server's connAccepted() also sits, so a socket that
  // dies mid-TLS is counted by neither.
  PlainCounter connectionsAccepted;
  // Gauge. Incremented with connectionsAccepted, decremented when the
  // connection's close callback runs.
  PlainCounter connectionsActive;
};

// Scopes the per-thread storage below to connection stats, so its slots are
// not interleaved with those of unrelated ThreadLocals.
struct ConnectionStatsTag {};

/**
 * Connection-layer counters, partitioned into one shard per IO thread.
 *
 * Attach an instance via FastThriftServer::setConnectionStats to have the
 * connection metrics handler wired into every acceptance pipeline; leaving it
 * unset adds no handler at all, so unattached servers pay nothing.
 *
 * Sharding is what makes the counters cheap rather than a contention
 * optimization over shared ones: a shard is only ever reached from the IO
 * thread that owns it, giving it exactly one writer and letting the counters
 * be plain integers instead of atomics.
 *
 * The cost is a reading discipline — a shard must be read from its own IO
 * thread, so a server-wide total means hopping onto each EventBase and
 * summing the copies. Counters are therefore eventually consistent, never
 * instantaneously exact. That is also why accessAllThreads() is not exposed:
 * it would hand a caller shards belonging to other threads.
 */
class ConnectionStats {
 public:
  /**
   * This thread's shard, constructed on first use.
   *
   * The connection handler keeps the returned reference for the life of its
   * EventBase. That is sound because a shard's address is fixed once created,
   * and because a handler cannot outlive either its IO thread or the server
   * holding this ConnectionStats.
   */
  ConnectionStatsShard& currentThreadShard() { return *shards_; }

 private:
  folly::ThreadLocal<ConnectionStatsShard, ConnectionStatsTag> shards_;
};

} // namespace apache::thrift::fast_thrift::connection
