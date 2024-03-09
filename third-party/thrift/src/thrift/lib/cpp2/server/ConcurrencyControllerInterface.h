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

#include <chrono>
#include <cstdint>
#include <memory>
#include <string_view>

#include <thrift/lib/cpp2/server/RequestCompletionCallback.h>
#include <thrift/lib/thrift/gen-cpp2/serverdbginfo_types.h>

namespace apache::thrift {

class ServerRequest;

// This common interface to all concurrency controllers. The details of
// construction are left to implementations and/or factories.
//
// The concurrency controller manages the number of active requests and
// limits it to a specified limit. It is expected to acquire requests from
// a request pile.
class ConcurrencyControllerInterface : public RequestCompletionCallback {
 public:
  // Set the limit for simultanous requests. 0 indicates unlimites. This is
  // thread safe. If it is increased the concurrency controller must attempt
  // to dispatch more requests to reach the limit. If it is reduced the
  // concurrency controller waits for enough existing requests to complete to
  // bring the number below the limit before starting a new request.
  virtual void setExecutionLimitRequests(uint64_t limit) = 0;

  // Get the limit on how many requests may be executed simultaneously. 0
  // indicates unlimited. This is thread safe.
  virtual uint64_t getExecutionLimitRequests() const = 0;

  virtual void setQpsLimit(uint64_t limit) = 0;

  virtual uint64_t getQpsLimit() const = 0;

  // Returns the current number of requests being processed by this concurrency
  // controller. This is only intended for monitoring. This is thread safe.
  virtual uint64_t requestCount() const = 0;

  // This must be called whenever a request is successfully enqueued to the
  // request pile associated with this ConcurrencyController (ie enqueue()
  // in RequestPileInterface returns a successful result).
  virtual void onEnqueued() = 0;

  using UserData = intptr_t;

  // This will be called when the request processing has finished.
  void onRequestFinished(ServerRequestData&) override;

  // Stops the concurrency controller. Stops dispatching new requests. This is
  // thread safe and does not block.
  virtual void stop() = 0;

  // This is for temporarily logging pending dequeue counts
  // in ParallelConcurrencyController
  virtual uint64_t numPendingDequeRequest() const { return 0; }

  virtual std::string describe() const = 0;

  virtual serverdbginfo::ConcurrencyControllerDbgInfo getDbgInfo() const {
    return {};
  }

  // ConcurrencyController can notify an observer when request execution is
  // completed
  class Observer {
   public:
    virtual ~Observer() {}
    virtual void onFinishExecution(ServerRequest& request) = 0;
  };

  virtual void setObserver(std::shared_ptr<Observer> observer) = 0;
};

} // namespace apache::thrift
