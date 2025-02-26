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
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/python/server/PythonAsyncProcessor.h>
#include <thrift/lib/python/server/python_async_processor_api.h> // @manual

namespace thrift::python {

namespace {

void do_import() {
  if (0 != import_thrift__python__server_impl__python_async_processor()) {
    throw std::runtime_error(
        "import thrift.python.server_impl.python_async_processor failed");
  }
}

auto get_deserialize_error_function(apache::thrift::ProtocolType protocol) {
  return protocol == apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
      ? apache::thrift::detail::ap::process_handle_exn_deserialization<
            apache::thrift::BinaryProtocolWriter>
      : apache::thrift::detail::ap::process_handle_exn_deserialization<
            apache::thrift::CompactProtocolWriter>;
}

} // namespace

std::unique_ptr<folly::IOBuf> PythonAsyncProcessor::getPythonMetadata() {
  [[maybe_unused]] static bool done = (do_import(), false);
  return getSerializedPythonMetadata(python_server_);
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::handlePythonServerCallback(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    std::unique_ptr<
        apache::thrift::HandlerCallback<std::unique_ptr<::folly::IOBuf>>>
        callback) {
  [[maybe_unused]] static bool done = (do_import(), false);
  auto [promise, future] =
      folly::makePromiseContract<std::unique_ptr<folly::IOBuf>>();
  handleServerCallback(
      functions_.at(context->getMethodName()).second,
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol,
      kind);
  return std::move(future).defer([callback = std::move(callback)](auto&& t) {
    callback->complete(std::move(t));
  });
}

folly::SemiFuture<folly::Unit>
PythonAsyncProcessor::handlePythonServerCallbackStreaming(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    std::unique_ptr<::apache::thrift::HandlerCallback<
        ::apache::thrift::ResponseAndServerStream<
            std::unique_ptr<::folly::IOBuf>,
            std::unique_ptr<::folly::IOBuf>>>> callback) {
  [[maybe_unused]] static bool done = (do_import(), false);
  auto [promise, future] =
      folly::makePromiseContract<::apache::thrift::ResponseAndServerStream<
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>>>();
  handleServerStreamCallback(
      functions_.at(context->getMethodName()).second,
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol,
      kind);
  return std::move(future).defer([callback = std::move(callback)](auto&& t) {
    callback->complete(std::move(t));
  });
}

folly::SemiFuture<folly::Unit>
PythonAsyncProcessor::handlePythonServerCallbackOneway(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    std::unique_ptr<apache::thrift::HandlerCallbackBase>&& callback) {
  [[maybe_unused]] static bool done = (do_import(), false);
  auto [promise, future] = folly::makePromiseContract<folly::Unit>();
  handleServerCallbackOneway(
      functions_.at(context->getMethodName()).second,
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol,
      kind);
  return std::move(future).defer([callback = std::move(callback)](auto&&) {});
}

void PythonAsyncProcessor::executeReadEventCallbacks(
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::ContextStack* ctxStack,
    apache::thrift::SerializedRequest& serializedRequest,
    apache::thrift::protocol::PROTOCOL_TYPES protocol) {
  if (ctxStack) {
    ctxStack->preRead();

    apache::thrift::SerializedMessage smsg;
    smsg.protocolType = protocol;
    smsg.buffer = serializedRequest.buffer.get();
    smsg.methodName = ctx->getMethodName();
    ctxStack->onReadData(smsg);

    ctxStack->postRead(
        nullptr,
        serializedRequest.buffer
            ->computeChainDataLength()); // TODO move this call to inside
                                         // the python code
  }
}

void PythonAsyncProcessor::processSerializedCompressedRequestWithMetadata(
    apache::thrift::ResponseChannelRequest::UniquePtr req,
    apache::thrift::SerializedCompressedRequest&& serializedCompressedRequest,
    const apache::thrift::AsyncProcessorFactory::MethodMetadata&
        untypedMethodMetadata,
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* context,
    folly::EventBase* eb,
    apache::thrift::concurrency::ThreadManager* tm) {
  const auto& methodMetadata =
      apache::thrift::AsyncProcessorHelper::expectMetadataOfType<
          PythonMetadata>(untypedMethodMetadata);

  // TODO just copying this from the rust thrift server, fetch this data
  // from the actual python server
  std::string interactionName;
  bool interactionFactoryMethod = false;
  auto serializedRequest = std::move(serializedCompressedRequest).uncompress();
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

  if (!(protocol ==
            apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL ||
        protocol ==
            apache::thrift::protocol::PROTOCOL_TYPES::T_COMPACT_PROTOCOL)) {
    req->sendErrorWrapped(
        apache::thrift::TApplicationException(
            "Thrift Python server only supports Binary and Compact Protocols."),
        kConnectionClosingErrorCode);
    return;
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

  auto executor = tm
      ? tm->getKeepAlive(
            context->getRequestExecutionScope(),
            apache::thrift::concurrency::ThreadManager::Source::INTERNAL)
      : nullptr;

  auto kind = methodMetadata.rpcKind.value();
  const char* serviceName = serviceName_.c_str();

  auto ctxStack = apache::thrift::ContextStack::create(
      this->getEventHandlersSharedPtr(),
      serviceName,
      functionFullNameMap_.at(context->getMethodName()).c_str(),
      context);

  try {
    executeReadEventCallbacks(
        context, ctxStack.get(), serializedRequest, protocol);
  } catch (...) {
    folly::exception_wrapper ew(std::current_exception());
    auto throw_func = get_deserialize_error_function(protocol);
    throw_func(
        ew,
        std::move(req),
        context,
        eb,
        functionFullNameMap_.at(context->getMethodName()).c_str());
    return;
  }

  // While this folly::makeSemiFuture().deferValue() may seem
  // unnecessary, without this deferValue, the call to
  // do_import(), defined at the top of this file,
  // which happens via the call to dispatchRequest() below
  // will crash with a null pointer derefence.
  // The hypothesis is that python is not yet initialized
  // and we chose not to go down that rabbit hole because
  // the current implementation matches what was already present.
  folly::makeSemiFuture()
      .deferValue([this,
                   protocol,
                   context,
                   eb,
                   executor,
                   serviceName,
                   kind,
                   req = std::move(req),
                   ctxStack = std::move(ctxStack),
                   serializedRequest = std::move(serializedRequest)](
                      auto&& /* unused */) mutable {
        if (!req) {
          return folly::makeSemiFuture();
        }

        if (!req->getShouldStartProcessing()) {
          // Ensure request is moved into HandlerCallback, so that request
          // is always destroyed on its EventBase thread
          if (eb) {
            apache::thrift::HandlerCallbackBase::releaseRequest(
                std::move(req), eb);
          }
          return folly::makeSemiFuture();
        }

        return dispatchRequest(
            protocol,
            context,
            eb,
            executor,
            apache::thrift::ServerRequestData{},
            std::move(req),
            std::move(ctxStack),
            serviceName,
            std::move(serializedRequest),
            kind);
      })
      .via(executor_);
}

void PythonAsyncProcessor::executeRequest(
    apache::thrift::ServerRequest&& request,
    const AsyncProcessorFactory::MethodMetadata& untypedMethodMetadata) {
  const auto& methodMetadata =
      apache::thrift::AsyncProcessorHelper::expectMetadataOfType<
          PythonMetadata>(untypedMethodMetadata);

  auto protocol =
      apache::thrift::detail::ServerRequestHelper::protocol(request);
  auto* ctx = request.requestContext();

  if (!(protocol ==
            apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL ||
        protocol ==
            apache::thrift::protocol::PROTOCOL_TYPES::T_COMPACT_PROTOCOL)) {
    request.request()->sendErrorWrapped(
        apache::thrift::TApplicationException(
            "Thrift Python server only supports Binary and Compact Protocols."),
        kConnectionClosingErrorCode);
    return;
  }

  const char* serviceName = serviceName_.c_str();
  auto ctxStack = apache::thrift::ContextStack::create(
      this->getEventHandlersSharedPtr(),
      serviceName,
      functionFullNameMap_.at(ctx->getMethodName()).c_str(),
      ctx);

  auto serializedRequest =
      std::move(apache::thrift::detail::ServerRequestHelper::compressedRequest(
                    request))
          .uncompress();

  auto* eb = apache::thrift::detail::ServerRequestHelper::eventBase(request);
  auto executor =
      apache::thrift::detail::ServerRequestHelper::executor(request);
  auto requestData = request.requestData();
  auto req =
      apache::thrift::detail::ServerRequestHelper::request(std::move(request));
  auto kind = methodMetadata.rpcKind;

  try {
    executeReadEventCallbacks(ctx, ctxStack.get(), serializedRequest, protocol);
  } catch (...) {
    folly::exception_wrapper ew(std::current_exception());
    auto throw_func = get_deserialize_error_function(protocol);
    throw_func(
        ew,
        std::move(req),
        ctx,
        eb,
        functionFullNameMap_.at(ctx->getMethodName()).c_str());
    return;
  }

  dispatchRequest(
      protocol,
      ctx,
      eb,
      executor,
      std::move(requestData),
      std::move(req),
      std::move(ctxStack),
      serviceName,
      std::move(serializedRequest),
      kind.value())
      .via(executor_);
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequest(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    folly::EventBase* eb,
    folly::Executor::KeepAlive<> executor,
    apache::thrift::ServerRequestData requestData,
    apache::thrift::ResponseChannelRequest::UniquePtr req,
    apache::thrift::ContextStack::UniquePtr ctxStack,
    const char* serviceName,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind) {
  const char* methodName = ctx->getMethodName().c_str();
  auto throw_wrapped =
      protocol == apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
      ? &detail::throw_wrapped<
            apache::thrift::BinaryProtocolReader,
            apache::thrift::BinaryProtocolWriter>
      : &detail::throw_wrapped<
            apache::thrift::CompactProtocolReader,
            apache::thrift::CompactProtocolWriter>;

  if (kind == apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE) {
    return handlePythonServerCallbackOneway(
        protocol,
        ctx,
        std::move(serializedRequest),
        kind,
        std::make_unique<apache::thrift::HandlerCallbackBase>(
            std::move(req),
            std::move(ctxStack),
            serviceName,
            serviceName, /* definingServiceName */
            methodName,
            nullptr,
            eb,
            executor,
            ctx,
            nullptr,
            nullptr,
            requestData));
  } else if (
      kind == apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE) {
    auto return_streaming =
        protocol == apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
        ? &detail::return_streaming<
              apache::thrift::BinaryProtocolReader,
              apache::thrift::BinaryProtocolWriter>
        : &detail::return_streaming<
              apache::thrift::CompactProtocolReader,
              apache::thrift::CompactProtocolWriter>;
    return handlePythonServerCallbackStreaming(
        protocol,
        ctx,
        std::move(serializedRequest),
        kind,
        std::make_unique<apache::thrift::HandlerCallback<
            ::apache::thrift::ResponseAndServerStream<
                std::unique_ptr<::folly::IOBuf>,
                std::unique_ptr<::folly::IOBuf>>>>(
            std::move(req),
            std::move(ctxStack),
            serviceName,
            serviceName, /* definingServiceName */
            methodName,
            return_streaming,
            throw_wrapped,
            ctx->getProtoSeqId(),
            eb,
            executor,
            ctx,
            nullptr,
            nullptr,
            requestData));
  } else {
    auto return_serialized =
        protocol == apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
        ? &detail::return_serialized<
              apache::thrift::BinaryProtocolReader,
              apache::thrift::BinaryProtocolWriter>
        : &detail::return_serialized<
              apache::thrift::CompactProtocolReader,
              apache::thrift::CompactProtocolWriter>;

    return handlePythonServerCallback(
        protocol,
        ctx,
        std::move(serializedRequest),
        kind,
        std::make_unique<
            apache::thrift::HandlerCallback<std::unique_ptr<::folly::IOBuf>>>(
            std::move(req),
            std::move(ctxStack),
            serviceName,
            serviceName, /* definingServiceName */
            methodName,
            return_serialized,
            throw_wrapped,
            ctx->getProtoSeqId(),
            eb,
            executor,
            ctx,
            nullptr,
            nullptr,
            requestData));
  }
}

} // namespace thrift::python
