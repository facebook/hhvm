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

#include <thrift/lib/python/server/server.h>
#include <thrift/lib/python/server/server_api.h> // @manual

namespace thrift {
namespace python {

namespace {

void do_import() {
  if (0 != import_thrift__python__server()) {
    throw std::runtime_error("import_thrift__python__server failed");
  }
}

} // namespace

void PythonAsyncProcessor::handlePythonServerCallback(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    folly::Promise<std::unique_ptr<folly::IOBuf>> promise,
    apache::thrift::SerializedRequest serializedRequest) {
  FOLLY_MAYBE_UNUSED static bool done = (do_import(), false);
  handleServerCallback(
      functions_.at(context->getMethodName()),
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol);
}

void PythonAsyncProcessor::handlePythonServerCallbackOneway(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    folly::Promise<folly::Unit> promise,
    apache::thrift::SerializedRequest serializedRequest) {
  FOLLY_MAYBE_UNUSED static bool done = (do_import(), false);
  handleServerCallbackOneway(
      functions_.at(context->getMethodName()),
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol);
}

} // namespace python
} // namespace thrift
