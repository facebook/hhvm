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
  class Observer {
   public:
    virtual ~Observer() {}
    virtual void onFinishExecution(ServerRequest& request) = 0;
  };

  // not thread-safe, should be called up front
  void setObserver(std::unique_ptr<Observer> ob);

  // Not thread-safe and should only be called once from the main thread
  static void setGlobalObserver(std::shared_ptr<Observer> observer);

  // thread-safe. grab the current global observer if any
  static Observer* getGlobalObserver();

 protected:
  void notifyOnFinishExecution(ServerRequest& request);

 private:
  std::unique_ptr<Observer> observer_{nullptr};
};

} // namespace apache::thrift
