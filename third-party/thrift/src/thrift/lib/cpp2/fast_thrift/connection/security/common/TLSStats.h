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

namespace apache::thrift::fast_thrift::connection::security {

/**
 * Per-EventBase block of the TLS pipeline's counters.
 *
 * Separate from ConnectionStatsShard because the two describe different
 * things and have different lifetimes of relevance: connection counters apply
 * to every server, these only to one that negotiates security at all. A
 * server under SSLPolicy::DISABLED builds no TLS pipeline, so it can simply
 * not have one of these rather than carry a block of counters that can only
 * ever read zero.
 *
 * Aligned so that two shards never share a cache line, for the same reason
 * the other shards are: shards are separately heap-allocated and could
 * otherwise land adjacent, reintroducing the cross-core interference the
 * single-writer design exists to avoid.
 */
struct alignas(folly::hardware_destructive_interference_size) TLSStatsShard {
  // Handshakes that completed. Counted where the TLS pipeline resolves a
  // connection, so a socket that died mid-handshake is not counted — matching
  // where the classic server counts too.
  PlainCounter tlsComplete;
  // Of those, the ones that resumed a session rather than running a full
  // handshake, and the ones where the peer presented a certificate.
  PlainCounter tlsResumption;
  PlainCounter tlsWithClientCert;
  // Connections given up on inside the TLS pipeline — handshake, peek, or
  // StopTLS downgrade. Every stage reports failure the same way, so this is
  // one number for "accepted but never served"; see StageFailure.h.
  PlainCounter tlsError;
};

// Scopes the per-thread storage below to TLS stats, so its slots are not
// interleaved with those of unrelated ThreadLocals.
struct TLSStatsTag {};

/**
 * TLS-layer counters, partitioned into one shard per IO thread.
 *
 * Attach an instance via FastThriftServer::setTLSStats to have the metrics
 * handler wired into every TLS pipeline; leaving it unset adds no handler at
 * all, so a server without it pays nothing — as does any server under
 * SSLPolicy::DISABLED, which has no TLS pipeline to wire it into.
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
class TLSStats {
 public:
  /**
   * This thread's shard, constructed on first use.
   *
   * The connection handler keeps the returned reference for the life of its
   * EventBase. That is sound because a shard's address is fixed once created,
   * and because a handler cannot outlive either its IO thread or the server
   * holding this TLSStats.
   */
  TLSStatsShard& currentThreadShard() { return *shards_; }

 private:
  folly::ThreadLocal<TLSStatsShard, TLSStatsTag> shards_;
};

} // namespace apache::thrift::fast_thrift::connection::security
