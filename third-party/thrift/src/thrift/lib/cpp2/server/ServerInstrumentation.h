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
#include <set>
#include <string>
#include <string_view>

#include <folly/Function.h>
#include <folly/concurrency/memory/PrimaryPtr.h>

namespace apache {
namespace thrift {

class ThriftServer;

namespace instrumentation {

constexpr std::string_view kThriftServerTrackerKey = "thrift_server";

class ServerTrackerRef {
 public:
  bool tryWithLock(
      folly::FunctionRef<void(std::string_view, ThriftServer&)> f) {
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
    ControlBlock(std::string_view k, ThriftServer& s) : key(k), server(&s) {}
    std::string_view key;
    ThriftServer* server;
  };

  explicit ServerTrackerRef(folly::PrimaryPtrRef<ControlBlock> ref)
      : ref_(std::move(ref)) {}
  folly::PrimaryPtrRef<ControlBlock> ref_;
};

class ServerTracker {
 public:
  ServerTracker(std::string_view key, ThriftServer& server);
  ~ServerTracker();

  ThriftServer& getServer() const { return server_; }

  ServerTrackerRef ref() const { return ServerTrackerRef{cb_.ref()}; }

  const std::string& getKey() const { return key_; }

 private:
  std::string key_;
  ThriftServer& server_;
  folly::PrimaryPtr<ServerTrackerRef::ControlBlock> cb_;
};

size_t getServerCount(std::string_view key);

void forEachServer(
    std::string_view key, folly::FunctionRef<void(ThriftServer&)>);

} // namespace instrumentation
} // namespace thrift
} // namespace apache
