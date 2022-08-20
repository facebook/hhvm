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

#include <stdint.h>

#include <memory>

#include <folly/io/IOBuf.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/cpp2/transport/core/ThriftChannelIf.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {

class ThriftServer;

/**
 * Server side Thrift processor.  Accepts calls from the channel,
 * calls the function handler, and finally calls back into the channel
 * object.
 *
 * Only one object of this type is created when initializing the
 * server.  This object handles all calls across all threads.
 */
class ThriftProcessor {
 public:
  explicit ThriftProcessor(ThriftServer& server);

  virtual ~ThriftProcessor() = default;

  ThriftProcessor(const ThriftProcessor&) = delete;
  ThriftProcessor& operator=(const ThriftProcessor&) = delete;

  // Called once for each RPC from a channel object.  After performing
  // some checks and setup operations, this schedules the function
  // handler on a worker thread.  "headers" and "payload" are passed
  // to the handler as parameters.  For RPCs with streaming requests,
  // "payload" contains the non-stream parameters of the function.
  // "channel" is used to call back with the response for single
  // (non-streaming) responses, and to manage stream objects for RPCs
  // with streaming.
  virtual void onThriftRequest(
      RequestRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> payload,
      std::shared_ptr<ThriftChannelIf> channel,
      std::unique_ptr<Cpp2ConnContext> connContext) noexcept;

 private:
  std::unique_ptr<AsyncProcessor> processor_;
  ThriftServer& server_;
};

} // namespace thrift
} // namespace apache
