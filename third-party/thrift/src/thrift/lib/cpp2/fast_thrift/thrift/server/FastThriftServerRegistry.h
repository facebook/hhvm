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

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include <folly/Function.h>
#include <folly/concurrency/memory/PrimaryPtr.h>

namespace apache::thrift::fast_thrift::thrift {

class FastThriftServer;

namespace instrumentation {

/**
 * Tracker key for FastThriftServer instances. Symmetric with legacy's
 * `apache::thrift::instrumentation::kThriftServerTrackerKey =
 * "thrift_server"` — chosen so that consumers iterating both legacy and
 * fast_thrift servers in the same process can disambiguate by key.
 */
constexpr std::string_view kFastThriftServerTrackerKey = "fast_thrift_server";

/**
 * Weak handle to a tracked server. Use `tryWithLock` to access the server
 * under a guarantee that the underlying ServerTracker (and therefore the
 * server) is still alive for the duration of the callback.
 *
 * Mirrors apache::thrift::instrumentation::ServerTrackerRef.
 */
class ServerTrackerRef {
 public:
  bool tryWithLock(
      folly::FunctionRef<void(std::string_view, FastThriftServer&)> f) {
    if (auto locked = ref_.lock()) {
      DCHECK(locked->server);
      f(locked->key, *locked->server);
      return true;
    }
    return false;
  }

 private:
  friend class ServerTracker;
  struct ControlBlock {
    ControlBlock(std::string_view k, FastThriftServer& s)
        : key(k), server(&s) {}
    std::string_view key;
    FastThriftServer* server;
  };

  explicit ServerTrackerRef(folly::PrimaryPtrRef<ControlBlock> ref)
      : ref_(std::move(ref)) {}
  folly::PrimaryPtrRef<ControlBlock> ref_;
};

/**
 * Per-server registration handle. Construct as a FastThriftServer data
 * member; the destructor blocks until all in-flight `forEachServer`
 * callbacks observing this server have returned, so the server is safe to
 * destroy as soon as ~ServerTracker returns.
 */
class ServerTracker {
 public:
  ServerTracker(std::string_view key, FastThriftServer& server);
  ~ServerTracker();

  ServerTracker(const ServerTracker&) = delete;
  ServerTracker& operator=(const ServerTracker&) = delete;
  ServerTracker(ServerTracker&&) = delete;
  ServerTracker& operator=(ServerTracker&&) = delete;

  FastThriftServer& getServer() const { return server_; }
  const std::string& getKey() const { return key_; }

  /**
   * Process-unique, monotonically increasing registration id.
   *
   * Distinguishes a fresh registration from a destroyed one that happened to
   * be allocated at the same address — which a consumer keyed on the server
   * pointer alone cannot do, and which would otherwise let per-server state
   * outlive the server it describes.
   */
  uint64_t id() const { return id_; }

  ServerTrackerRef ref() const { return ServerTrackerRef{cb_.ref()}; }

 private:
  uint64_t id_;
  std::string key_;
  FastThriftServer& server_;
  folly::PrimaryPtr<ServerTrackerRef::ControlBlock> cb_;
};

/// Number of FastThriftServers currently registered under `key`.
size_t getServerCount(std::string_view key);

/**
 * Invoke `f` for each FastThriftServer registered under `key`. Callbacks
 * run under a read lock on the registry — keep them short. Do not issue
 * RPCs or call `start`/`stop` from inside the callback.
 */
void forEachServer(
    std::string_view key, folly::FunctionRef<void(FastThriftServer&)> f);

/**
 * As forEachServer, additionally passing each server's registration id. Same
 * callback constraints apply — the registry read lock is held throughout, and
 * a ServerTracker being destroyed blocks until the callback returns.
 */
void forEachServerWithId(
    std::string_view key,
    folly::FunctionRef<void(uint64_t, FastThriftServer&)> f);

} // namespace instrumentation

} // namespace apache::thrift::fast_thrift::thrift
