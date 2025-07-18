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

#include <thrift/lib/cpp2/async/processor/ServerInterface.h>

namespace apache::thrift {

thread_local RequestParams ServerInterface::requestParams_;

void ServerInterface::setEventBase(folly::EventBase* eb) {
  folly::RequestEventBase::set(eb);
  requestParams_.eventBase_ = eb;
}

void ServerInterface::BlockingThreadManager::add(folly::Func f) {
  try {
    if (threadManagerKa_) {
      std::shared_ptr<concurrency::Runnable> task =
          concurrency::FunctionRunner::create(std::move(f));
      threadManagerKa_->add(
          std::move(task),
          std::chrono::milliseconds(kTimeout).count() /* deprecated */,
          0,
          false);
    } else {
      executorKa_->add(std::move(f));
    }
    return;
  } catch (...) {
    LOG(FATAL) << "Failed to schedule a task within timeout: "
               << folly::exceptionStr(folly::current_exception());
  }
}

bool ServerInterface::BlockingThreadManager::keepAliveAcquire() noexcept {
  auto keepAliveCount = keepAliveCount_.fetch_add(1, std::memory_order_relaxed);
  // We should never increment from 0
  DCHECK(keepAliveCount > 0);
  return true;
}

void ServerInterface::BlockingThreadManager::keepAliveRelease() noexcept {
  auto keepAliveCount = keepAliveCount_.fetch_sub(1, std::memory_order_acq_rel);
  DCHECK(keepAliveCount >= 1);
  if (keepAliveCount == 1) {
    delete this;
  }
}

concurrency::PRIORITY ServerInterface::getRequestPriority(
    Cpp2RequestContext* ctx, concurrency::PRIORITY prio) {
  concurrency::PRIORITY callPriority = ctx->getCallPriority();
  return callPriority == concurrency::N_PRIORITIES ? prio : callPriority;
}

folly::Executor::KeepAlive<> ServerInterface::getInternalKeepAlive() {
  if (getThreadManager_deprecated()) {
    return getThreadManager_deprecated()->getKeepAlive(
        getRequestContext()->getRequestExecutionScope(),
        apache::thrift::concurrency::ThreadManager::Source::INTERNAL);
  } else {
    return folly::Executor::getKeepAliveToken(getHandlerExecutor());
  }
}

} // namespace apache::thrift
