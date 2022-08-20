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

#include <optional>

#include <folly/io/async/Request.h>
#include <thrift/lib/cpp2/server/ConcurrencyControllerInterface.h>

namespace apache::thrift {

// ConcurrencyControllerBase
// This class provides all the helper functionalities of ConcurrencyController
class ConcurrencyControllerBase : public ConcurrencyControllerInterface {
 public:
  struct PerRequestStats {
    std::chrono::steady_clock::time_point queueBegin;
    std::chrono::steady_clock::time_point workBegin;
    std::chrono::steady_clock::time_point workEnd;
  };

  class Observer {
   public:
    virtual ~Observer() {}

    // callback right before request is executed
    virtual PerRequestStats onExecute(ServerRequest& /* request  */) {
      return PerRequestStats();
    }

    // callback right after the request is finished execution
    // This is more of the handler code completion instead
    // of the request hits its end of liftcycle
    virtual void onFinishExecution(
        folly::RequestContext* /* context  */, PerRequestStats& /* stats  */) {}
  };

  std::optional<PerRequestStats> onExecute(ServerRequest& /* request */);

  void onFinishExecution(
      folly::RequestContext* /* context  */, PerRequestStats& /* stats */);

  // not thread-safe
  // Should be called up front
  void setObserver(std::unique_ptr<Observer> ob);

  // set the global observer of the concurrency controller
  // This method is not thread-safe and should only be called
  // once from the main thread
  static void setGlobalObserver(std::shared_ptr<Observer> observer);

  // thread-safe. grab the current global observer if any
  static Observer* getGlobalObserver();

 private:
  std::unique_ptr<Observer> observer_{nullptr};
};

} // namespace apache::thrift
