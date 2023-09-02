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
#include <thrift/lib/python/server/util.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace thrift {
namespace python {

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
        executor(std::move(executor)),
        serviceName_(std::move(serviceName)) {
    for (const auto& function : functions) {
      functionFullNameMap_.insert(
          {function.first, fmt::format("{}.{}", serviceName_, function.first)});
    }
  }

  using ProcessFunc = void (PythonAsyncProcessor::*)(
      apache::thrift::ResponseChannelRequest::UniquePtr,
      apache::thrift::SerializedCompressedRequest&&,
      apache::thrift::Cpp2RequestContext* context,
      folly::EventBase* eb,
      apache::thrift::concurrency::ThreadManager* tm);
  struct ProcessFuncs {
    ProcessFunc compact;
    ProcessFunc binary;
  };
  struct PythonMetadata final
      : public apache::thrift::AsyncProcessorFactory::MethodMetadata {
    explicit PythonMetadata(ProcessFuncs funcs) : processFuncs(funcs) {}

    ProcessFuncs processFuncs;
  };

  std::unique_ptr<folly::IOBuf> getPythonMetadata();

  void getServiceMetadata(
      apache::thrift::metadata::ThriftServiceMetadataResponse& response)
      override {
    std::unique_ptr<folly::IOBuf> buf = folly::via(this->executor, [this] {
                                          return getPythonMetadata();
                                        }).get();
    apache::thrift::BinarySerializer::deserialize<
        apache::thrift::metadata::ThriftServiceMetadataResponse>(
        buf.get(), response);
  }

  void handlePythonServerCallback(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      folly::Promise<std::unique_ptr<folly::IOBuf>> promise,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind);

  void handlePythonServerCallbackStreaming(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      folly::Promise<::apache::thrift::ResponseAndServerStream<
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>>> promise,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind);

  void handlePythonServerCallbackOneway(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      folly::Promise<folly::Unit> promise,
      apache::thrift::SerializedRequest serializedRequest,
      apache::thrift::RpcKind kind);

  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::SerializedCompressedRequest&& serializedRequest,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&
          untypedMethodMetadata,
      apache::thrift::protocol::PROTOCOL_TYPES protType,
      apache::thrift::Cpp2RequestContext* context,
      folly::EventBase* eb,
      apache::thrift::concurrency::ThreadManager* tm) override {
    const auto& methodMetadata =
        apache::thrift::AsyncProcessorHelper::expectMetadataOfType<
            PythonMetadata>(untypedMethodMetadata);

    // TODO just copying this from the rust thrift server, fetch this data
    // from the actual python server
    std::string interactionName;
    bool interactionFactoryMethod = false;
    if (context->getInteractionId()) {
      std::string_view serviceName{context->getMethodName()};
      serviceName = serviceName.substr(0, serviceName.find("."));
      if (auto interactionCreate = context->getInteractionCreate()) {
        if (interactionCreate->interactionName_ref()->view() == serviceName) {
          interactionName = serviceName;
          interactionFactoryMethod = false;
        } else {
          interactionName = interactionCreate->interactionName_ref()->str();
          interactionFactoryMethod = true;
        }
      } else {
        interactionName = serviceName;
        interactionFactoryMethod = false;
      }
    }

    if (!setUpRequestProcessing(
            req,
            context,
            eb,
            tm,
            functions_.at(context->getMethodName())
                .first, // TODO check if this will error out
            this,
            interactionName,
            interactionFactoryMethod)) {
      return;
    }
    ProcessFunc pfn;
    switch (protType) {
      case apache::thrift::protocol::T_BINARY_PROTOCOL: {
        pfn = methodMetadata.processFuncs.binary;
        break;
      }
      case apache::thrift::protocol::T_COMPACT_PROTOCOL: {
        pfn = methodMetadata.processFuncs.compact;
        break;
      }
      default:
        LOG(ERROR) << "invalid protType: " << folly::to_underlying(protType);
        return;
    }
    (this->*pfn)(std::move(req), std::move(serializedRequest), context, eb, tm);
  }

  template <
      typename ProtocolIn_,
      typename ProtocolOut_,
      apache::thrift::RpcKind kind>
  void genericProcessor(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::SerializedCompressedRequest&& serializedCompressedRequest,
      apache::thrift::Cpp2RequestContext* ctx,
      folly::EventBase* eb,
      apache::thrift::concurrency::ThreadManager* tm) {
    ProtocolIn_ prot;
    auto serializedRequest =
        std::move(serializedCompressedRequest).uncompress();

    static_assert(ProtocolIn_::protocolType() == ProtocolOut_::protocolType());
    apache::thrift::ContextStack::UniquePtr ctxStack(this->getContextStack(
        serviceName_.c_str(),
        functionFullNameMap_.at(ctx->getMethodName()).c_str(),
        ctx));

    if (ctxStack) {
      ctxStack->preRead();

      apache::thrift::SerializedMessage smsg;
      smsg.protocolType = ProtocolIn_::protocolType();
      smsg.buffer = serializedRequest.buffer.get();
      smsg.methodName = ctx->getMethodName();
      ctxStack->onReadData(smsg);

      ctxStack->postRead(
          nullptr,
          serializedRequest.buffer
              ->computeChainDataLength()); // TODO move this call to inside the
                                           // python code
    }

    folly::via(
        this->executor,
        [this,
         prot,
         ctx,
         eb,
         tm,
         req = std::move(req),
         ctxStack = std::move(ctxStack),
         serializedRequest = std::move(serializedRequest)]() mutable {
          if (req && req->getShouldStartProcessing()) {
            if constexpr (
                kind == apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE) {
              auto callback =
                  std::make_unique<apache::thrift::HandlerCallbackBase>(
                      std::move(req),
                      std::move(ctxStack),
                      nullptr,
                      eb,
                      tm,
                      ctx);
              auto [promise, future] =
                  folly::makePromiseContract<folly::Unit>();
              handlePythonServerCallbackOneway(
                  prot.protocolType(),
                  ctx,
                  std::move(promise),
                  std::move(serializedRequest),
                  kind);
              std::move(future)
                  .via(this->executor)
                  .thenTry([callback = std::move(callback)](auto&& /* t */) {});
            } else if constexpr (
                kind ==
                apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE) {
              auto callback = std::make_unique<apache::thrift::HandlerCallback<
                  ::apache::thrift::ResponseAndServerStream<
                      std::unique_ptr<::folly::IOBuf>,
                      std::unique_ptr<::folly::IOBuf>>>>(
                  std::move(req),
                  std::move(ctxStack),
                  detail::return_streaming<ProtocolIn_, ProtocolOut_>,
                  detail::throw_wrapped<ProtocolIn_, ProtocolOut_>,
                  ctx->getProtoSeqId(),
                  eb,
                  tm,
                  ctx);
              auto [promise, future] = folly::makePromiseContract<
                  ::apache::thrift::ResponseAndServerStream<
                      std::unique_ptr<::folly::IOBuf>,
                      std::unique_ptr<::folly::IOBuf>>>();
              handlePythonServerCallbackStreaming(
                  prot.protocolType(),
                  ctx,
                  std::move(promise),
                  std::move(serializedRequest),
                  kind);
              std::move(future)
                  .via(this->executor)
                  .thenTry([callback = std::move(callback)](auto&& t) {
                    callback->complete(std::move(t));
                  });
            } else {
              auto callback = std::make_unique<apache::thrift::HandlerCallback<
                  std::unique_ptr<::folly::IOBuf>>>(
                  std::move(req),
                  std::move(ctxStack),
                  detail::return_serialized<ProtocolIn_, ProtocolOut_>,
                  detail::throw_wrapped<ProtocolIn_, ProtocolOut_>,
                  ctx->getProtoSeqId(),
                  eb,
                  tm,
                  ctx);
              auto [promise, future] =
                  folly::makePromiseContract<std::unique_ptr<folly::IOBuf>>();
              handlePythonServerCallback(
                  prot.protocolType(),
                  ctx,
                  std::move(promise),
                  std::move(serializedRequest),
                  kind);
              std::move(future)
                  .via(this->executor)
                  .thenTry([callback = std::move(callback)](auto&& t) {
                    callback->complete(std::move(t));
                  });
            }
          }
        });
  }

  // Dud method for GeneratedAsyncProcessor
  const char* getServiceName() override {
    LOG(WARNING) << "PythonAsyncProcessor::getServiceName called unexpectedly";
    return "PythonService";
  }

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

  static const PythonAsyncProcessor::ProcessFuncs getSingleFunc() {
    return singleFunc_;
  }

  static const PythonAsyncProcessor::ProcessFuncs getOnewayFunc() {
    return onewayFunc_;
  }

  static const PythonAsyncProcessor::ProcessFuncs getStreamFunc() {
    return streamFunc_;
  }

 private:
  PyObject* python_server_;
  std::unordered_map<std::string, std::string> functionFullNameMap_;
  const FunctionMapType& functions_;
  folly::Executor::KeepAlive<> executor;
  std::string serviceName_;
  static inline const PythonAsyncProcessor::ProcessFuncs singleFunc_{
      &PythonAsyncProcessor::genericProcessor<
          apache::thrift::CompactProtocolReader,
          apache::thrift::CompactProtocolWriter,
          apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE>,
      &PythonAsyncProcessor::genericProcessor<
          apache::thrift::BinaryProtocolReader,
          apache::thrift::BinaryProtocolWriter,
          apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE>};
  static inline const PythonAsyncProcessor::ProcessFuncs onewayFunc_{
      &PythonAsyncProcessor::genericProcessor<
          apache::thrift::CompactProtocolReader,
          apache::thrift::CompactProtocolWriter,
          apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE>,
      &PythonAsyncProcessor::genericProcessor<
          apache::thrift::BinaryProtocolReader,
          apache::thrift::BinaryProtocolWriter,
          apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE>};
  static inline const PythonAsyncProcessor::ProcessFuncs streamFunc_{
      &PythonAsyncProcessor::genericProcessor<
          apache::thrift::CompactProtocolReader,
          apache::thrift::CompactProtocolWriter,
          apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE>,
      &PythonAsyncProcessor::genericProcessor<
          apache::thrift::BinaryProtocolReader,
          apache::thrift::BinaryProtocolWriter,
          apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE>};
};

} // namespace python
} // namespace thrift
