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

#include <folly/executors/SerialExecutor.h>
#include <thrift/lib/cpp2/async/AsyncProcessorHelper.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>

namespace apache::thrift {

namespace {
std::string describeRequestExecutionMode(
    ParallelConcurrencyController::RequestExecutionMode requestExecutionMode) {
  switch (requestExecutionMode) {
    case ParallelConcurrencyController::RequestExecutionMode::Serial:
      return "Serial";
    case ParallelConcurrencyController::RequestExecutionMode::Parallel:
      return "Parallel";
  }
}
} // namespace

void ParallelConcurrencyControllerBase::setExecutionLimitRequests(
    uint64_t limit) {
  executionLimit_.store(limit);
  // When the limit is changed, we call trySchedule() to fill the gap
  // of new limit and the old (if new > old)
  while (trySchedule()) {
  }
}

void ParallelConcurrencyControllerBase::onEnqueued() {
  trySchedule(true);
}

// This function decrement the requestInExecution counter
// and try to schedule again in case of an edge scenario:
// When requestInExecution == executionLimit, before we decrement,
// there is another request pushed in, this request will sit in the queue
// without being pulled if we don't schedule another task
// And if the dequeue failed for some reason, we still decremented the
// pendingCounter, so we are adding it back in here as well
void ParallelConcurrencyControllerBase::onExecuteFinish(bool dequeueSuccess) {
  for (;;) {
    auto countersOld = counters_.load();
    auto counters = countersOld;
    --counters.requestInExecution;
    counters.pendingDequeCalls += dequeueSuccess ? 0 : 1;
    if (!counters_.compare_exchange_weak(countersOld, counters)) {
      continue;
    }
    break;
  }
  trySchedule();
}

void ParallelConcurrencyControllerBase::onRequestFinished(ServerRequestData&) {
  onExecuteFinish(true);
}

// Pseudocode:
// if (enqueue) pendingDequeCalls++
// if (pendingDequeCalls &&  < executionLimit) {
//   pendingDequeCalls--;
//   requestInExecution++;
//   executor.add();
//   return true;
// } else {
//   return false;
// }
bool ParallelConcurrencyControllerBase::trySchedule(bool onEnqueued) {
  for (;;) {
    auto countersOld = counters_.load();
    auto counters = countersOld;
    // It's possible that executionLimit gets changed during the call
    // but since it is pretty rare we don't compare test it in this
    // implementation
    auto currentLimit = executionLimit_.load();

    // Only when the caller is onEnqueue() will we increment pendingDequeCalls
    // to indicate that we have more coming sitting in the queue
    if (onEnqueued) {
      ++counters.pendingDequeCalls;
    }

    // If there are requests sitting in the queue and we are under limit
    // we update the counter to grab an execution ticket
    // (++counters.requestInExecution) and grab a dequeue ticket
    // (--counters.pendingDequeCalls)
    if (counters.pendingDequeCalls &&
        counters.requestInExecution < currentLimit) {
      ++counters.requestInExecution;
      --counters.pendingDequeCalls;

      // If the swap succeeded we schedule the task on the executor
      if (!counters_.compare_exchange_weak(countersOld, counters)) {
        continue;
      }

      scheduleOnExecutor();
      return true;
    }

    // This branch only happens when execution is out of limit
    // in that case we would like to increase pendingDequeCalls only
    if (onEnqueued && !counters_.compare_exchange_weak(countersOld, counters)) {
      continue;
    }

    return false;
  }
}

namespace {
template <typename Executor, typename Function>
FOLLY_ALWAYS_INLINE void scheduleOnExecutorInner(
    Executor& executor, Function&& function) {
  if (executor.getNumPriorities() > 1) {
    // By default we have 2 prios, external requests should go to
    // lower priority queue to yield to the internal ones
    executor.addWithPriority(
        std::forward<Function>(function), folly::Executor::LO_PRI);
  } else {
    executor.add(std::forward<Function>(function));
  }
}
} // namespace

void ParallelConcurrencyController::scheduleOnExecutor() {
  switch (requestExecutionMode_) {
    case RequestExecutionMode::Parallel: {
      return scheduleOnExecutorInner(
          executor_, [this]() { executeRequest(pile_.dequeue()); });
    }
    case RequestExecutionMode::Serial: {
      auto keepAlive = folly::SerialExecutor::create(
          folly::Executor::getKeepAliveToken(executor_));
      auto& executor = *keepAlive.get();
      return scheduleOnExecutorInner(
          executor, [this, ka = std::move(keepAlive)]() mutable {
            auto req = pile_.dequeue();
            if (req) {
              apache::thrift::detail::ServerRequestHelper::setExecutor(
                  req.value(), std::move(ka));
            }
            executeRequest(std::move(req));
          });
    }
  }
  folly::assume_unreachable();
}

void ParallelConcurrencyControllerBase::executeRequest(
    std::optional<ServerRequest> req) {
  if (req) {
    ServerRequest& serverRequest = req.value();

    serverRequest.setConcurrencyControllerNotification(this);
    serverRequest.setRequestPileNotification(&pile_);

    // Only continue when the request has not
    // expired (not queue-timeouted)
    if (!serverRequest.request()->isOneway() &&
        !serverRequest.request()->getShouldStartProcessing()) {
      using namespace apache::thrift::detail;
      if (onExpireFunction_) {
        onExpireFunction_(serverRequest);
      }
      auto eb = ServerRequestHelper::eventBase(serverRequest);
      auto request = ServerRequestHelper::request(std::move(serverRequest));
      HandlerCallbackBase::releaseRequest(std::move(request), eb);
      return;
    }

    if (onExecuteFunction_) {
      onExecuteFunction_(serverRequest);
    }
    serverRequest.requestData().setRequestExecutionBegin();
    AsyncProcessorHelper::executeRequest(std::move(*req));
    serverRequest.requestData().setRequestExecutionEnd();

    notifyOnFinishExecution(serverRequest);
    return;

  } else {
    // We do this to avoid the case where after we
    // do an empty dequeue() there is a request that
    // gets pushed in.
    onExecuteFinish(false);
    return;
  }
}

void ParallelConcurrencyControllerBase::processExpiredRequest(
    ServerRequest&& request) {
  using namespace apache::thrift::detail;
  if (onExpireFunction_) {
    onExpireFunction_(request);
  }
  auto eb = ServerRequestHelper::eventBase(request);
  auto req = ServerRequestHelper::request(std::move(request));
  HandlerCallbackBase::releaseRequest(std::move(req), eb);

  for (;;) {
    auto countersOld = counters_.load();
    auto counters = countersOld;
    counters.pendingDequeCalls -= 1;
    if (counters_.compare_exchange_weak(countersOld, counters)) {
      break;
    }
  }
}

void ParallelConcurrencyControllerBase::stop() {}

std::string ParallelConcurrencyController::describe() const {
  return fmt::format(
      "{{ParallelConcurrencyController executionLimit={}, requestExecutionMode={}}}",
      executionLimit_.load(),
      describeRequestExecutionMode(requestExecutionMode_));
}

serverdbginfo::ConcurrencyControllerDbgInfo
ParallelConcurrencyController::getDbgInfo() const {
  serverdbginfo::ConcurrencyControllerDbgInfo info;
  info.name() = folly::demangle(typeid(*this));
  info.concurrencyLimit() = executionLimit_.load();
  info.qpsLimit() = 0; // no QPS limit, concurrency limit is used
  return info;
}

} // namespace apache::thrift
