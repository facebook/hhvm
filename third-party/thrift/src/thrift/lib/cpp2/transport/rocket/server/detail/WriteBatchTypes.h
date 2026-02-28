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

#include <vector>

#include <folly/net/NetOps.h>

#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionObserver.h>

namespace apache::thrift::rocket {

// Types that were previously nested in RocketServerConnection or WriteBatcher
using FdsAndOffsets = std::vector<std::pair<folly::SocketFds, size_t>>;

struct WriteBatchContext {
  // the counts of completed requests in each inflight write
  size_t requestCompleteCount{0};
  // the counts of valid sendCallbacks in each inflight write
  std::vector<apache::thrift::MessageChannel::SendCallbackPtr> sendCallbacks;
  // the WriteEvent objects associated with each write in the batch
  std::vector<RocketServerConnectionObserver::WriteEvent> writeEvents;
  // the raw byte offset at the beginning and end of the inflight write
  RocketServerConnectionObserver::WriteEventBatchContext writeEventsContext;
};

} // namespace apache::thrift::rocket
