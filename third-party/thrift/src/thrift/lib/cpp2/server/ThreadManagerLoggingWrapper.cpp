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

#include <thrift/lib/cpp2/server/ThreadManagerLoggingWrapper.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

namespace apache::thrift {

folly::once_flag ThreadManagerLoggingWrapper::recordFlag_{};

void ThreadManagerLoggingWrapper::recordStackTrace(std::string funcName) const {
  if (server_) {
    folly::call_once(
        ThreadManagerLoggingWrapper::recordFlag_,
        [server_ = server_,
         funcName = std::move(funcName),
         shouldLog = shouldLog_]() mutable {
          if (!shouldLog) {
            return;
          }
          auto& actions = server_->getRuntimeServerActions();
          actions.executorToThreadManagerUnexpectedFunctionName =
              std::move(funcName);
          // let's reused the old event instead of creating a new one
          if (auto server = server_) {
            THRIFT_SERVER_EVENT(executor_thread_manager_unexpected_calls)
                .log(*server);
          }
        });
  }
}

void ThreadManagerLoggingWrapper::checkResourcePoolsEnabled() const {
  if (priorityThreadManager_ == nullptr && resourcePoolsEnabled_) {
    LOG(FATAL)
        << "This deprecated ThreadManager method, which is dependent on PriorityThreadManager, is not supported after migration to Resource Pools. "
        << "Please migrate to ResourcePools API";
  }
}
} // namespace apache::thrift
