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
#include <thrift/lib/python/server/PythonAsyncProcessor.h>
#include <thrift/lib/python/server/PythonAsyncProcessorFactory.h>
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
  }
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
      [[maybe_unused]] static bool done = (do_import(), false);
      handleLifecycleCallback(
          func, getLifecycleFuncName(funcType), std::move(promise));
      return std::move(future);
    }
  }
  return folly::makeSemiFuture();
}

PythonAsyncProcessorFactory::CreateMethodMetadataResult
PythonAsyncProcessorFactory::createMethodMetadata() {
  AsyncProcessorFactory::MethodMetadataMap result;
  using PythonMetadataForRpcKind = std::unordered_map<
      apache::thrift::RpcKind,
      std::shared_ptr<PythonAsyncProcessor::PythonMetadata>>;
  static const PythonMetadataForRpcKind kPythonMetadataForRpcKind = []() {
    PythonMetadataForRpcKind pythonMetadataForRpcKind;
    for (auto rpcKind :
         {apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
          apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE,
          apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE}) {
      pythonMetadataForRpcKind[rpcKind] =
          std::make_shared<PythonAsyncProcessor::PythonMetadata>(rpcKind);
    }
    return pythonMetadataForRpcKind;
  }();

  for (const auto& [methodName, function] : functions_) {
    switch (function.first) {
      case apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
      case apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE:
      case apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE:
        result.emplace(
            methodName, kPythonMetadataForRpcKind.at(function.first));
        break;
      case apache::thrift::RpcKind::SINK:
        // Python doesn't support sink yet. Explictly
        // add this case for visibility when it does support it.
        break;
    }
  }

  return result;
}

} // namespace python
} // namespace thrift
