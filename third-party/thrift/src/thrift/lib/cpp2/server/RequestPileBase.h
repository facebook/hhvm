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

#include <memory>
#include <folly/executors/QueueObserver.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>

namespace apache::thrift {

class RequestPileBase : public RequestPileInterface {
 protected:
  explicit RequestPileBase(std::string name);

  void setUpQueueObservers();

  void onEnqueued(ServerRequest& req);

  // This function set up the threadIdCollector
  // and calls queueObserver onDequeued() for
  // logging queue lag counters.
  // Note: This should only be called when dequeue
  // happens on the CPU workers
  void onDequeued(ServerRequest& req);

  serverdbginfo::RequestPileDbgInfo getDbgInfo() const override;

 private:
  std::string resourcePoolName_;

  // Below are for queue observer setups
  constexpr static unsigned defaultQueueObserverNumPrio = 1;
  // A WorkerProvider instance which can be used to collect stack traces
  // from threads consuming from lagging queues.
  std::shared_ptr<folly::ThreadIdWorkerProvider> threadIdCollector_{
      std::make_shared<folly::ThreadIdWorkerProvider>()};
  // A queue observer that monitors counters like queue lag
  // One approach is we have one observer per priority
  // But for now let only use one observer per ResourcePool
  // Because the purpose of this is more of monitoring whether
  // a thread in a thread pool is getting stuck
  std::unique_ptr<folly::QueueObserver> queueObserver_;

  void registerThreadId();
};

} // namespace apache::thrift
