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

#include <thrift/lib/python/server/event_handler.h>

#include <folly/io/async/Request.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>

namespace apache::thrift::python {

std::string getRequestId() {
  if (auto rctx = folly::RequestContext::get()) {
    if (auto rootId = rctx->getRootId();
        RequestsRegistry::isThriftRootId(rootId)) {
      return RequestsRegistry::getRequestId(rootId);
    }
  }
  return {};
}

PythonServerEventHandler::PythonServerEventHandler(
    folly::Executor* executor, AddressHandler address_handler)
    : executor_(executor), address_handler_(std::move(address_handler)) {}

void PythonServerEventHandler::preServe(const folly::SocketAddress* address) {
  executor_->add(
      [addr = *address, this]() mutable { address_handler_(std::move(addr)); });
}

} // namespace apache::thrift::python
