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
#include <thrift/lib/python/server/PythonAsyncProcessor.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift::python {

class PythonAsyncProcessorFactory
    : public apache::thrift::AsyncProcessorFactory,
      public apache::thrift::ServiceHandlerBase {
 public:
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

  CreateMethodMetadataResult createMethodMetadata() override;

  /**
   * This factory function is an extension of the python
   * PythonAsyncProcessorFactory.create() in python to handle the C++ side of
   * the python PythonAsyncProcessorFactory.create().
   *
   * Callers must use this factory to create a PythonAsyncProcessorFactory
   * instance.
   */
  static std::shared_ptr<PythonAsyncProcessorFactory> create(
      PyObject* python_server,
      FunctionMapType functions,
      std::vector<PyObject*> lifecycleFuncs,
      folly::Executor::KeepAlive<> executor,
      std::string serviceName);

 private:
  folly::SemiFuture<folly::Unit> callLifecycle(LifecycleFunc);

  PyObject* python_server_;
  const FunctionMapType functions_;
  const std::vector<PyObject*> lifecycleFuncs_;
  folly::Executor::KeepAlive<> executor;
  std::string serviceName_;

  using PythonMetadataForRpcKind = std::unordered_map<
      apache::thrift::RpcKind,
      std::shared_ptr<PythonAsyncProcessor::PythonMetadata>>;
  PythonMetadataForRpcKind pythonMetadataForRpcKind_;

  /**
   * Callers must use the create() factory function to create a
   * PythonAsyncProcessorFactory instance. Make the constructor
   * private to enforce this.
   */
  PythonAsyncProcessorFactory(
      PyObject* python_server,
      FunctionMapType functions,
      std::vector<PyObject*> lifecycleFuncs,
      folly::Executor::KeepAlive<> executor,
      std::string serviceName,
      PythonMetadataForRpcKind pythonMetadataForRpcKind)
      : python_server_(python_server),
        functions_(std::move(functions)),
        lifecycleFuncs_(std::move(lifecycleFuncs)),
        executor(std::move(executor)),
        serviceName_(std::move(serviceName)),
        pythonMetadataForRpcKind_(std::move(pythonMetadataForRpcKind)) {}
};

} // namespace apache::thrift::python
