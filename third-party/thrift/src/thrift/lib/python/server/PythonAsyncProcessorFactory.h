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

#include <exception>
#include <map>
#include <memory>
#include <string>
#include <Python.h>
#include <glog/logging.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/gen/service_tcc.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/python/server/server.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace thrift {
namespace python {

class PythonAsyncProcessorFactory
    : public apache::thrift::AsyncProcessorFactory,
      public apache::thrift::ServiceHandlerBase {
 public:
  PythonAsyncProcessorFactory(
      PyObject* python_server,
      FunctionMapType functions,
      std::vector<PyObject*> lifecycleFuncs,
      folly::Executor::KeepAlive<> executor,
      std::string serviceName)
      : python_server_(python_server),
        functions_(std::move(functions)),
        lifecycleFuncs_(std::move(lifecycleFuncs)),
        executor(std::move(executor)),
        serviceName_(std::move(serviceName)) {}

  folly::SemiFuture<folly::Unit> semifuture_onStartServing() override;
  folly::SemiFuture<folly::Unit> semifuture_onStopRequested() override;

  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    return std::make_unique<PythonAsyncProcessor>(
        python_server_, functions_, executor, serviceName_);
  }

  std::vector<apache::thrift::ServiceHandlerBase*> getServiceHandlers()
      override {
    return {this};
  }

  CreateMethodMetadataResult createMethodMetadata() override {
    AsyncProcessorFactory::MethodMetadataMap result;
    const auto processFunc =
        std::make_shared<PythonAsyncProcessor::PythonMetadata>(
            PythonAsyncProcessor::getSingleFunc());
    const auto onewayFunc =
        std::make_shared<PythonAsyncProcessor::PythonMetadata>(
            PythonAsyncProcessor::getOnewayFunc());
    const auto streamFunc =
        std::make_shared<PythonAsyncProcessor::PythonMetadata>(
            PythonAsyncProcessor::getStreamFunc());

    for (const auto& [methodName, function] : functions_) {
      switch (function.first) {
        case apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
          result.emplace(methodName, processFunc);
          break;
        case apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE:
          result.emplace(methodName, onewayFunc);
          break;
        case apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE:
          result.emplace(methodName, streamFunc);
          break;
        case apache::thrift::RpcKind::SINK:
          // Leaving this commented out on purpose, as python doesn't support
          // sink methods yet, but we'll still need this switch branch for when
          // it does

          // result.emplace(methodName, processFunc); // TODO sink
          break;
      }
    }

    return result;
  }

 private:
  folly::SemiFuture<folly::Unit> callLifecycle(LifecycleFunc);

  PyObject* python_server_;
  const FunctionMapType functions_;
  const std::vector<PyObject*> lifecycleFuncs_;
  folly::Executor::KeepAlive<> executor;
  std::string serviceName_;
};

} // namespace python
} // namespace thrift
