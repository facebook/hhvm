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

#include <memory>
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

std::unique_ptr<folly::IOBuf> PythonAsyncProcessor::getPythonMetadata() {
  FOLLY_MAYBE_UNUSED static bool done = (do_import(), false);
  return getSerializedPythonMetadata(python_server_);
}

void PythonAsyncProcessor::handlePythonServerCallback(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    folly::Promise<std::unique_ptr<folly::IOBuf>> promise,
    apache::thrift::SerializedRequest serializedRequest) {
  FOLLY_MAYBE_UNUSED static bool done = (do_import(), false);
  handleServerCallback(
      functions_.at(context->getMethodName()).second,
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
      functions_.at(context->getMethodName()).second,
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol);
}

enum class LifecycleFunc {
  ON_START_SERVING = 0,
  ON_STOP_REQUESTED = 1,
};

std::string getLifecycleFuncName(LifecycleFunc func) {
  switch (func) {
    case LifecycleFunc::ON_START_SERVING: {
      return "onStartServing";
    }
    case LifecycleFunc::ON_STOP_REQUESTED: {
      return "onStopRequested";
    }
  };
}

folly::SemiFuture<folly::Unit>
PythonAsyncProcessorFactory::semifuture_onStartServing() {
  return callLifecycle(LifecycleFunc::ON_START_SERVING);
}

folly::SemiFuture<folly::Unit>
PythonAsyncProcessorFactory::semifuture_onStopRequested() {
  return callLifecycle(LifecycleFunc::ON_STOP_REQUESTED);
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessorFactory::callLifecycle(
    LifecycleFunc funcType) {
  std::size_t ind = (std::size_t)funcType;
  if (ind < lifecycleFuncs_.size()) {
    if (auto func = lifecycleFuncs_[ind]) {
      auto [promise, future] = folly::makePromiseContract<folly::Unit>();
      FOLLY_MAYBE_UNUSED static bool done = (do_import(), false);
      handleLifecycleCallback(
          func, getLifecycleFuncName(funcType), std::move(promise));
      return std::move(future);
    }
  }
  return folly::makeSemiFuture();
}

} // namespace python
} // namespace thrift
