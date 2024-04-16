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

#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/SEParallelConcurrencyController.h>

namespace apache::thrift {

void SEParallelConcurrencyController::scheduleOnExecutor() {
  auto keepAlive = folly::Executor::getKeepAliveToken(executor_);
  auto executor = folly::SmallSerialExecutor::create(std::move(keepAlive));

  auto task = [this, executor]() mutable {
    auto req = pile_.dequeue();
    if (req) {
      apache::thrift::detail::ServerRequestHelper::setExecutor(
          req.value(), std::move(executor));
    }
    executeRequest(std::move(req));
  };

  if (executor->getNumPriorities() > 1) {
    // By default we have 2 prios, external requests should go to
    // lower priority queue to yield to the internal ones
    executor->addWithPriority(std::move(task), folly::Executor::LO_PRI);
  } else {
    executor->add(std::move(task));
  }
}

std::string SEParallelConcurrencyController::describe() const {
  return fmt::format(
      "{{SEParallelConcurrencyController executionLimit={}}}",
      executionLimit_.load());
}

serverdbginfo::ConcurrencyControllerDbgInfo
SEParallelConcurrencyController::getDbgInfo() const {
  serverdbginfo::ConcurrencyControllerDbgInfo info;
  info.name() = folly::demangle(typeid(*this));
  info.qpsLimit() = getQpsLimit();
  info.concurrencyLimit() = getExecutionLimitRequests();
  return info;
}

} // namespace apache::thrift
