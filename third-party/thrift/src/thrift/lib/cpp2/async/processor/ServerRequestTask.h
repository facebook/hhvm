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

#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/processor/ServerRequest.h>

namespace apache::thrift {

class ServerRequestTask : public concurrency::Runnable, public InteractionTask {
 public:
  explicit ServerRequestTask(ServerRequest&& req) : req_(std::move(req)) {}
  ~ServerRequestTask() override;

  void failWith(folly::exception_wrapper ex, std::string exCode) override;

  void setTile(TilePtr&& tile) override;

  void run() override {
    // This override exists because these tasks are stored as Runnable in the
    // interaction queues.
    LOG(FATAL) << "Should never be called";
  }

  void acceptIntoResourcePool(int8_t priority) override;

  friend class Tile;
  friend class TilePromise;
  friend class GeneratedAsyncProcessorBase;

 private:
  ServerRequest req_;
};

} // namespace apache::thrift
