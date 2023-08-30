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
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace thrift {
namespace python {

enum class LifecycleFunc;

constexpr size_t kMaxUexwSize = 1024;

class PythonUserException : public std::exception {
 public:
  PythonUserException(
      std::string type, std::string reason, std::unique_ptr<folly::IOBuf> buf)
      : type_(std::move(type)),
        reason_(std::move(reason)),
        buf_(std::move(buf)) {}
  PythonUserException(const PythonUserException& ex)
      : type_(ex.type_), reason_(ex.reason_), buf_(ex.buf_->clone()) {}

  PythonUserException& operator=(const PythonUserException& ex) {
    type_ = ex.type_;
    reason_ = ex.reason_;
    buf_ = ex.buf_->clone();
    return *this;
  }

  const std::string& type() const { return type_; }
  const std::string& reason() const { return reason_; }
  const folly::IOBuf* buf() const { return buf_.get(); }
  const char* what() const noexcept override { return reason_.c_str(); }

 private:
  std::string type_;
  std::string reason_;
  std::unique_ptr<folly::IOBuf> buf_;
};

class PythonAsyncProcessor : public apache::thrift::AsyncProcessor {
 public:
  PythonAsyncProcessor(
      PyObject* python_server,
      const std::map<
          std::string,
          std::pair<apache::thrift::RpcKind, PyObject*>>& functions,
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
      apache::thrift::SerializedRequest serializedRequest);

  void handlePythonServerCallbackOneway(
      apache::thrift::ProtocolType protocol,
      apache::thrift::Cpp2RequestContext* context,
      folly::Promise<folly::Unit> promise,
      apache::thrift::SerializedRequest serializedRequest);

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

  template <typename ProtocolIn_, typename ProtocolOut_>
  apache::thrift::ContextStack::UniquePtr preProcessRequest(
      apache::thrift::SerializedRequest& serializedRequest,
      apache::thrift::Cpp2RequestContext* ctx) {
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

    return ctxStack;
  }

  template <typename ProtocolIn_, typename ProtocolOut_>
  void genericProcessor(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::SerializedCompressedRequest&& serializedCompressedRequest,
      apache::thrift::Cpp2RequestContext* ctx,
      folly::EventBase* eb,
      apache::thrift::concurrency::ThreadManager* tm) {
    ProtocolIn_ prot;
    auto serializedRequest =
        std::move(serializedCompressedRequest).uncompress();
    apache::thrift::ContextStack::UniquePtr ctxStack =
        preProcessRequest<ProtocolIn_, ProtocolOut_>(serializedRequest, ctx);

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
            auto callback = std::make_unique<apache::thrift::HandlerCallback<
                std::unique_ptr<::folly::IOBuf>>>(
                std::move(req),
                std::move(ctxStack),
                return_serialized<ProtocolIn_, ProtocolOut_>,
                throw_wrapped<ProtocolIn_, ProtocolOut_>,
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
                std::move(serializedRequest));
            std::move(future)
                .via(this->executor)
                .thenTry([callback = std::move(callback)](
                             folly::Try<std::unique_ptr<folly::IOBuf>>&& t) {
                  callback->complete(std::move(t));
                });
          }
        });
  }

  template <typename ProtocolIn_, typename ProtocolOut_>
  void onewayProcessor(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::SerializedCompressedRequest&& serializedCompressedRequest,
      apache::thrift::Cpp2RequestContext* ctx,
      folly::EventBase* eb,
      apache::thrift::concurrency::ThreadManager* tm) {
    ProtocolIn_ prot;
    auto serializedRequest =
        std::move(serializedCompressedRequest).uncompress();
    apache::thrift::ContextStack::UniquePtr ctxStack =
        preProcessRequest<ProtocolIn_, ProtocolOut_>(serializedRequest, ctx);

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
            auto callback =
                std::make_unique<apache::thrift::HandlerCallbackBase>(
                    std::move(req), std::move(ctxStack), nullptr, eb, tm, ctx);
            auto [promise, future] = folly::makePromiseContract<folly::Unit>();
            handlePythonServerCallbackOneway(
                prot.protocolType(),
                ctx,
                std::move(promise),
                std::move(serializedRequest));
            std::move(future)
                .via(this->executor)
                .thenTry([callback = std::move(callback)](
                             folly::Try<folly::Unit>&& /* t */) {});
          }
        });
  }

  static const PythonAsyncProcessor::ProcessFuncs getSingleFunc() {
    return singleFunc_;
  }

  static const PythonAsyncProcessor::ProcessFuncs getOnewayFunc() {
    return onewayFunc_;
  }

 private:
  PyObject* python_server_;
  std::unordered_map<std::string, std::string> functionFullNameMap_;
  const std::map<std::string, std::pair<apache::thrift::RpcKind, PyObject*>>&
      functions_;
  folly::Executor::KeepAlive<> executor;
  std::string serviceName_;
  static inline const PythonAsyncProcessor::ProcessFuncs singleFunc_{
      &PythonAsyncProcessor::genericProcessor<
          apache::thrift::CompactProtocolReader,
          apache::thrift::CompactProtocolWriter>,
      &PythonAsyncProcessor::genericProcessor<
          apache::thrift::BinaryProtocolReader,
          apache::thrift::BinaryProtocolWriter>};
  static inline const PythonAsyncProcessor::ProcessFuncs onewayFunc_{
      &PythonAsyncProcessor::onewayProcessor<
          apache::thrift::CompactProtocolReader,
          apache::thrift::CompactProtocolWriter>,
      &PythonAsyncProcessor::onewayProcessor<
          apache::thrift::BinaryProtocolReader,
          apache::thrift::BinaryProtocolWriter>};

  template <class ProtocolIn, class ProtocolOut>
  static apache::thrift::SerializedResponse return_serialized(
      apache::thrift::ContextStack* ctx, const ::folly::IOBuf& _return) {
    folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
    ProtocolOut prot;

    // Preallocate small buffer headroom for transports metadata & framing.
    constexpr size_t kHeadroomBytes = 128;
    auto buf = folly::IOBuf::create(kHeadroomBytes);
    buf->advance(kHeadroomBytes);
    queue.append(std::move(buf));

    prot.setOutput(&queue, 0);
    if (ctx) {
      ctx->preWrite();
    }
    queue.append(_return);
    if (ctx) {
      apache::thrift::SerializedMessage smsg;
      smsg.protocolType = prot.protocolType();
      smsg.methodName = "";
      smsg.buffer = queue.front();
      ctx->onWriteData(smsg);
    }
    DCHECK_LE(
        queue.chainLength(),
        static_cast<size_t>(std::numeric_limits<int>::max()));
    if (ctx) {
      ctx->postWrite(folly::to_narrow(queue.chainLength()));
    }
    return apache::thrift::SerializedResponse{queue.move()};
  }

  template <class ProtocolIn_, class ProtocolOut_>
  static void throw_wrapped(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      int32_t protoSeqId,
      apache::thrift::ContextStack* ctx,
      folly::exception_wrapper ew,
      apache::thrift::Cpp2RequestContext* reqCtx) {
    if (!ew) {
      return;
    }
    {
      if (ew.with_exception([&](const PythonUserException& e) {
            auto header = reqCtx->getHeader();
            if (!header) {
              return;
            }

            // TODO: (ffrancet) error kind overrides currently usupported,
            // by python, add kHeaderExMeta header support when it is
            header->setHeader(
                std::string(apache::thrift::detail::kHeaderUex), e.type());
            const std::string reason = e.reason();
            header->setHeader(
                std::string(apache::thrift::detail::kHeaderUexw),
                reason.size() > kMaxUexwSize ? reason.substr(0, kMaxUexwSize)
                                             : reason);

            ProtocolOut_ prot;
            auto response =
                return_serialized<ProtocolIn_, ProtocolOut_>(ctx, *e.buf());
            auto payload = std::move(response).extractPayload(
                req->includeEnvelope(),
                prot.protocolType(),
                protoSeqId,
                apache::thrift::MessageType::T_REPLY,
                reqCtx->getMethodName().c_str());
            payload.transform(reqCtx->getHeader()->getWriteTransforms());
            return req->sendReply(std::move(payload));
          })) {
      } else {
        apache::thrift::detail::ap::process_throw_wrapped_handler_error<
            ProtocolOut_>(
            ew, std::move(req), reqCtx, ctx, reqCtx->getMethodName().c_str());
      }
    }
  }
};

class PythonAsyncProcessorFactory
    : public apache::thrift::AsyncProcessorFactory,
      public apache::thrift::ServiceHandlerBase {
 public:
  PythonAsyncProcessorFactory(
      PyObject* python_server,
      std::map<std::string, std::pair<apache::thrift::RpcKind, PyObject*>>
          functions,
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

    for (const auto& [methodName, function] : functions_) {
      switch (function.first) {
        case apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
          result.emplace(methodName, processFunc);
          break;
        case apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE:
          result.emplace(methodName, onewayFunc);
          break;
        case apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE:
          result.emplace(methodName, processFunc); // TODO stream
          break;
        case apache::thrift::RpcKind::SINK:
          result.emplace(methodName, processFunc); // TODO sink
          break;
      }
    }

    return result;
  }

 private:
  folly::SemiFuture<folly::Unit> callLifecycle(LifecycleFunc);

  PyObject* python_server_;
  const std::map<std::string, std::pair<apache::thrift::RpcKind, PyObject*>>
      functions_;
  const std::vector<PyObject*> lifecycleFuncs_;
  folly::Executor::KeepAlive<> executor;
  std::string serviceName_;
};

} // namespace python
} // namespace thrift
