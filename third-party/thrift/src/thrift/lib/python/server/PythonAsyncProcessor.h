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
#include <thrift/lib/python/server/response_helpers.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift::python {

enum class LifecycleFunc;

using FunctionMapType =
    std::map<std::string, std::pair<apache::thrift::RpcKind, PyObject*>>;

class PythonAsyncProcessor : public apache::thrift::GeneratedAsyncProcessorBase,
                             public apache::thrift::ServerInterface {
 public:
  PythonAsyncProcessor(
      PyObject* python_server,
      const FunctionMapType& functions,
      folly::Executor::KeepAlive<> executor,
      std::string serviceName)
      : python_server_(python_server),
        functions_(functions),
        executor_(std::move(executor)),
        serviceName_(std::move(serviceName)) {
    for (const auto& function : functions) {
      functionFullNameMap_.insert(
          {function.first, fmt::format("{}.{}", serviceName_, function.first)});
    }
  }

  struct PythonMetadata final
      : public apache::thrift::AsyncProcessorFactory::MethodMetadata {
    PythonMetadata(
        ExecutorType executor,
        InteractionType interaction,
        apache::thrift::RpcKind rpcKind)
        : MethodMetadata(
              executor,
              interaction,
              rpcKind,
              apache::thrift::concurrency::NORMAL,
              std::nullopt,
              false) {}
  };

  std::unique_ptr<folly::IOBuf> getPythonMetadata();

  void getServiceMetadata(
      apache::thrift::metadata::ThriftServiceMetadataResponse& response)
      override {
    std::unique_ptr<folly::IOBuf> buf = folly::via(this->executor_, [this] {
                                          return getPythonMetadata();
                                        }).get();
    apache::thrift::BinarySerializer::deserialize<
        apache::thrift::metadata::ThriftServiceMetadataResponse>(
        buf.get(), response);
  }

  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr,
      apache::thrift::SerializedCompressedRequest&&,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&,
      apache::thrift::protocol::PROTOCOL_TYPES,
      apache::thrift::Cpp2RequestContext*,
      folly::EventBase*,
      apache::thrift::concurrency::ThreadManager*) override {
    LOG(FATAL)
        << "processSerializedCompressedRequestWithMetadata support has been "
        << "removed from PythonAsyncProcessor";
  }

  void executeRequest(
      apache::thrift::ServerRequest&& request,
      const AsyncProcessorFactory::MethodMetadata& untypedMethodMetadata)
      override;

  // Dud method for GeneratedAsyncProcessor
  std::string_view getServiceName() override { return serviceName_; }

  // Dud method for ServerInterface
  std::string_view getGeneratedName() const override {
    LOG(WARNING)
        << "PythonAsyncProcessor::getGeneratedName called unexpectedly";
    return "PythonService";
  }

  // Dud method for ServerInterface
  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    LOG(WARNING) << "PythonAsyncProcessor::getProcessor called unexpectedly";
    return nullptr;
  }

 private:
  PyObject* python_server_;
  std::unordered_map<std::string, std::string> functionFullNameMap_;
  const FunctionMapType& functions_;
  folly::Executor::KeepAlive<> executor_;
  std::string serviceName_;

  folly::SemiFuture<folly::Unit> handlePythonServerCallback(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      apache::thrift::HandlerCallback<std::unique_ptr<::folly::IOBuf>>::Ptr
          callback);

  folly::SemiFuture<folly::Unit> handlePythonServerCallbackStreaming(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndServerStream<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> handlePythonServerCallbackSink(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndSinkConsumer<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> handlePythonServerCallbackBidi(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndStreamTransformation<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> handlePythonServerCallbackOneway(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      apache::thrift::HandlerCallbackBase::Ptr callback);

  folly::SemiFuture<folly::Unit> dispatchRequest(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      folly::EventBase* eb,
      folly::Executor::KeepAlive<> executor,
      apache::thrift::ServerRequestData requestData,
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::ContextStack::UniquePtr ctxStack,
      const char* serviceName,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind);

  folly::SemiFuture<folly::Unit> dispatchRequestOneway(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      apache::thrift::HandlerCallbackBase::Ptr callback);

  folly::SemiFuture<folly::Unit> dispatchRequestStreaming(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndServerStream<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> dispatchRequestSink(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndSinkConsumer<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> dispatchRequestBidi(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      ::apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndStreamTransformation<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr callback);

  folly::SemiFuture<folly::Unit> dispatchRequestResponse(
      apache::thrift::protocol::PROTOCOL_TYPES protocol,
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind,
      apache::thrift::HandlerCallback<std::unique_ptr<::folly::IOBuf>>::Ptr
          callback);

  void executeReadEventCallbacks(
      apache::thrift::Cpp2RequestContext* ctx,
      apache::thrift::ContextStack* ctxStack,
      apache::thrift::SerializedRequest& serializedRequest,
      apache::thrift::protocol::PROTOCOL_TYPES protocol);
};

} // namespace apache::thrift::python
