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
#include <optional>
#include <string>
#include <string_view>

#include <folly/Executor.h>
#include <folly/Synchronized.h>

#include <thrift/lib/cpp2/async/AsyncProcessorHelper.h>
#include <thrift/lib/cpp2/server/ConcurrencyControllerInterface.h>
#include <thrift/lib/cpp2/server/IResourcePoolAcceptor.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>

namespace apache::thrift {

// The ResourcePool is a single resource for executing requests. It comprises
// a request pile, an executor and a concurrency controller.
//
// The currently supported configurations are all three (for async request
// processing) or none (for synchronous request processing - denoted by thread =
// 'eb' in thrift idl).
//
// The request pile and concurrency controller are dedicated to a resource pool
// but the executor may be shared amongst multiple resource pools if desired.
class ResourcePool : public IResourcePoolAcceptor {
 public:
  // It is prefereble that the executor used in a resource pool has at least
  // this many priorities so that we can deprioritize tasks that start new
  // requests (compared to tasks that are continuations of existing requests).
  static constexpr unsigned int kPreferredExecutorNumPriorities = 3;

  ~ResourcePool() override;

  // Access to the request pile if it exists.
  std::optional<std::reference_wrapper<RequestPileInterface>> requestPile() {
    auto result = requestPile_.get();
    if (result) {
      return std::ref(*result);
    }
    return std::nullopt;
  }

  // Access to executor if it exists.
  std::optional<std::reference_wrapper<folly::Executor>> executor() {
    auto result = executor_.get();
    if (result) {
      return std::ref(*result);
    }
    return std::nullopt;
  }

  // Access tp executor as shared pointer if it exists.
  std::optional<std::shared_ptr<folly::Executor>> sharedPtrExecutor() {
    if (executor_) {
      return executor_;
    }
    return std::nullopt;
  }

  // Access to concurrency controller if it exists.
  std::optional<std::reference_wrapper<ConcurrencyControllerInterface>>
  concurrencyController() {
    auto result = concurrencyController_.get();
    if (result) {
      return std::ref(*result);
    }
    return std::nullopt;
  }

  // Get the name of this pool.
  std::string_view name() { return name_; }

  // Accept a request into the resource pool. If an empty optional is returned
  // the request is consumed (moved from), otherwise it is left intact.
  //
  // Once a request has been accepted the only outcomes should be
  // timeout/expired or executed.
  std::optional<ServerRequestRejection> accept(
      ServerRequest&& request) override;

  // Stop the resource pool. Prevent the concurrency controller from scheduling
  // new requests and join the threads in the executor to ensure all requests
  // have completed.
  void stop();

  std::string describe() const;

 private:
  friend class ResourcePoolSet;
  ResourcePool(
      std::unique_ptr<RequestPileInterface>&& requestPile,
      std::shared_ptr<folly::Executor> executor,
      std::unique_ptr<ConcurrencyControllerInterface>&& concurrencyController,
      std::string_view name);

  std::unique_ptr<RequestPileInterface> requestPile_;
  std::shared_ptr<folly::Executor> executor_;
  std::unique_ptr<ConcurrencyControllerInterface> concurrencyController_;
  std::string name_;
};
} // namespace apache::thrift
