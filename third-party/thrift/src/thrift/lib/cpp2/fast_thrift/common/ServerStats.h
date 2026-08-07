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

#include <thrift/lib/cpp2/fast_thrift/common/Stats.h>

namespace apache::thrift::fast_thrift {

// Scopes the per-thread storage below to server stats, so its slots are not
// interleaved with those of unrelated ThreadLocals.
struct ServerStatsTag {};

/**
 * Server-wide counters, partitioned into one ServerStatsShard per IO thread.
 *
 * Attach an instance via FastThriftServer::setStats to have the rocket and
 * thrift metrics handlers wired into every connection; leaving it unset adds
 * no handlers at all, so unattached servers pay nothing.
 *
 * Sharding here is not a contention optimization layered over shared
 * counters — it is what makes the counters cheap in the first place. A shard
 * is only ever handed to connections running on the IO thread that owns it,
 * which gives each shard exactly one writer thread and lets the counters be
 * plain integers instead of atomics.
 *
 * The cost of that is a reading discipline: a shard must be read from its own
 * IO thread. Reading one from anywhere else is a data race, so a consumer
 * wanting a server-wide total has to hop onto each EventBase, copy out that
 * shard's values, and sum the copies. Counters are consequently eventually
 * consistent, never instantaneously exact. That discipline is also why
 * accessAllThreads() is not exposed here: it would hand a caller shards
 * belonging to threads other than its own.
 */
class ServerStats {
 public:
  /**
   * This thread's shard, constructed on first use.
   *
   * Metrics handlers keep the returned reference for the life of their
   * connection. That is sound because a shard's address is fixed once
   * created, and because a connection cannot outlive either the IO thread it
   * runs on or the server holding this ServerStats.
   */
  ServerStatsShard& currentThreadShard() { return *shards_; }

 private:
  folly::ThreadLocal<ServerStatsShard, ServerStatsTag> shards_;
};

} // namespace apache::thrift::fast_thrift
