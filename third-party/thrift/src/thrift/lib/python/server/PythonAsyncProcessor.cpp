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

#include <folly/python/error.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/python/server/PythonAsyncProcessor.h>
#include <thrift/lib/python/server/python_async_processor_api.h> // @manual

namespace apache::thrift::python {

using apache::thrift::detail::processServiceInterceptorsOnRequest;
using apache::thrift::detail::ServiceInterceptorOnRequestArguments;
using apache::thrift::detail::shouldProcessServiceInterceptorsOnRequest;

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

// C++ ServiceInterceptors (i.e., those installed via cpp Service Framework)
// are invoked without serializing arguments.
ServiceInterceptorOnRequestArguments emptyInterceptorsArguments() {
  static std::tuple empty = std::make_tuple();
  return ServiceInterceptorOnRequestArguments(empty);
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
    apache::thrift::HandlerCallback<std::unique_ptr<::folly::IOBuf>>::Ptr
        callback) {
  [[maybe_unused]] static bool done = (do_import(), false);
  auto [promise, future] =
      folly::makePromiseContract<std::unique_ptr<folly::IOBuf>>();
  const int retcode = handleServerCallback(
      functions_.at(context->getMethodName()).second,
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol,
      kind);
  if (retcode != 0) {
    DCHECK(PyErr_Occurred());
    // converts python error to thrown std::runtime_error
    folly::python::handlePythonError(
        "PythonAsyncProcessor::handlePythonServerCallback: ");
  }
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
    ::apache::thrift::HandlerCallback<::apache::thrift::ResponseAndServerStream<
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  [[maybe_unused]] static bool done = (do_import(), false);
  auto [promise, future] =
      folly::makePromiseContract<::apache::thrift::ResponseAndServerStream<
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>>>();
  const int retcode = handleServerStreamCallback(
      functions_.at(context->getMethodName()).second,
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol,
      kind);
  if (retcode != 0) {
    DCHECK(PyErr_Occurred());
    // converts python error to thrown std::runtime_error
    folly::python::handlePythonError(
        "PythonAsyncProcessor::handlePythonServerCallbackStreaming: ");
  }
  return std::move(future).defer([callback = std::move(callback)](auto&& t) {
    callback->complete(std::move(t));
  });
}

folly::SemiFuture<folly::Unit>
PythonAsyncProcessor::handlePythonServerCallbackSink(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<::apache::thrift::ResponseAndSinkConsumer<
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  [[maybe_unused]] static bool done = (do_import(), false);
  auto [promise, future] =
      folly::makePromiseContract<::apache::thrift::ResponseAndSinkConsumer<
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>>>();
  const int retcode = handleServerSinkCallback(
      functions_.at(context->getMethodName()).second,
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol,
      kind);
  if (retcode != 0) {
    DCHECK(PyErr_Occurred());
    // converts python error to thrown std::runtime_error
    folly::python::handlePythonError(
        "PythonAsyncProcessor::handlePythonServerCallbackSink: ");
  }
  return std::move(future).defer([callback = std::move(callback)](auto&& t) {
    callback->complete(std::move(t));
  });
}

folly::SemiFuture<folly::Unit>
PythonAsyncProcessor::handlePythonServerCallbackBidi(
    apache::thrift::ProtocolType protocol,
    apache::thrift::Cpp2RequestContext* context,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<
        ::apache::thrift::ResponseAndStreamTransformation<
            std::unique_ptr<::folly::IOBuf>,
            std::unique_ptr<::folly::IOBuf>,
            std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  [[maybe_unused]] static bool done = (do_import(), false);
  auto [promise, future] = folly::makePromiseContract<
      ::apache::thrift::ResponseAndStreamTransformation<
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>,
          std::unique_ptr<::folly::IOBuf>>>();
  const int retcode = handleServerBidiCallback(
      functions_.at(context->getMethodName()).second,
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol,
      kind);
  if (retcode != 0) {
    DCHECK(PyErr_Occurred());
    // converts python error to thrown std::runtime_error
    folly::python::handlePythonError(
        "PythonAsyncProcessor::handlePythonServerCallbackBidi: ");
  }
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
    apache::thrift::HandlerCallbackBase::Ptr callback) {
  [[maybe_unused]] static bool done = (do_import(), false);
  auto [promise, future] = folly::makePromiseContract<folly::Unit>();
  const int retcode = handleServerCallbackOneway(
      functions_.at(context->getMethodName()).second,
      serviceName_ + "." + context->getMethodName(),
      context,
      std::move(promise),
      std::move(serializedRequest),
      protocol,
      kind);
  if (retcode != 0) {
    DCHECK(PyErr_Occurred());
    // converts python error to thrown std::runtime_error
    folly::python::handlePythonError(
        "PythonAsyncProcessor::handlePythonServerCallbackOneway: ");
  }
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

  // This folly::makeSemiFuture().deferValue()
  // ensures that the dispatchRequest(),
  // which imports the cython module that must happen
  // on the python thread, runs in the python thread.
  folly::makeSemiFuture()
      .deferValue([this,
                   protocol,
                   ctx,
                   eb,
                   executor,
                   serviceName,
                   kind,
                   requestData = std::move(requestData),
                   req = std::move(req),
                   ctxStack = std::move(ctxStack),
                   serializedRequest = std::move(serializedRequest)](
                      auto&& /* unused */) mutable {
        return dispatchRequest(
            protocol,
            ctx,
            eb,
            executor,
            std::move(requestData),
            std::move(req),
            std::move(ctxStack),
            serviceName,
            std::move(serializedRequest),
            kind.value());
      })
      .via(executor_);
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequestOneway(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    apache::thrift::HandlerCallbackBase::Ptr callback) {
  if (!shouldProcessServiceInterceptorsOnRequest(*callback)) {
    return handlePythonServerCallbackOneway(
        protocol, ctx, std::move(serializedRequest), kind, std::move(callback));
  }
  return processServiceInterceptorsOnRequest(
             *callback, emptyInterceptorsArguments())
      .semi()
      // see discussion below about why we don't use `defer`
      .deferValue([this,
                   protocol,
                   ctx,
                   request = std::move(serializedRequest),
                   kind,
                   callback](auto&&) mutable {
        return handlePythonServerCallbackOneway(
            protocol, ctx, std::move(request), kind, std::move(callback));
      });
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequestStreaming(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<::apache::thrift::ResponseAndServerStream<
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  if (!shouldProcessServiceInterceptorsOnRequest(*callback)) {
    return handlePythonServerCallbackStreaming(
        protocol, ctx, std::move(serializedRequest), kind, std::move(callback));
  }
  return processServiceInterceptorsOnRequest(
             *callback, emptyInterceptorsArguments())
      .semi()
      // see discussion below about why we don't use `defer`
      .deferValue([this,
                   protocol,
                   ctx,
                   request = std::move(serializedRequest),
                   kind,
                   callback](auto&&) mutable {
        return handlePythonServerCallbackStreaming(
            protocol, ctx, std::move(request), kind, std::move(callback));
      });
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequestSink(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<::apache::thrift::ResponseAndSinkConsumer<
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  if (!shouldProcessServiceInterceptorsOnRequest(*callback)) {
    return handlePythonServerCallbackSink(
        protocol, ctx, std::move(serializedRequest), kind, std::move(callback));
  }
  return processServiceInterceptorsOnRequest(
             *callback, emptyInterceptorsArguments())
      .semi()
      // see discussion below about why we don't use `defer`
      .deferValue([this,
                   protocol,
                   ctx,
                   request = std::move(serializedRequest),
                   kind,
                   callback](auto&&) mutable {
        return handlePythonServerCallbackSink(
            protocol, ctx, std::move(request), kind, std::move(callback));
      });
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequestBidi(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    ::apache::thrift::HandlerCallback<
        ::apache::thrift::ResponseAndStreamTransformation<
            std::unique_ptr<::folly::IOBuf>,
            std::unique_ptr<::folly::IOBuf>,
            std::unique_ptr<::folly::IOBuf>>>::Ptr callback) {
  if (!shouldProcessServiceInterceptorsOnRequest(*callback)) {
    return handlePythonServerCallbackBidi(
        protocol, ctx, std::move(serializedRequest), kind, std::move(callback));
  }
  return processServiceInterceptorsOnRequest(
             *callback, emptyInterceptorsArguments())
      .semi()
      // see discussion below about why we don't use `defer`
      .deferValue([this,
                   protocol,
                   ctx,
                   request = std::move(serializedRequest),
                   kind,
                   callback](auto&&) mutable {
        return handlePythonServerCallbackBidi(
            protocol, ctx, std::move(request), kind, std::move(callback));
      });
}

folly::SemiFuture<folly::Unit> PythonAsyncProcessor::dispatchRequestResponse(
    apache::thrift::protocol::PROTOCOL_TYPES protocol,
    apache::thrift::Cpp2RequestContext* ctx,
    apache::thrift::SerializedRequest serializedRequest,
    apache::thrift::RpcKind kind,
    HandlerCallback<std::unique_ptr<folly::IOBuf>>::Ptr callback) {
  if (!shouldProcessServiceInterceptorsOnRequest(*callback)) {
    return handlePythonServerCallback(
        protocol, ctx, std::move(serializedRequest), kind, std::move(callback));
  }

  return processServiceInterceptorsOnRequest(
             *callback, emptyInterceptorsArguments())
      .semi()
      // It may appear that we're discarding exception result of onRequest
      // interceptor, but it's actually caught via throw_wrapped, which
      // invokes sendException to report the callback completed with exception,
      // thereby invoking the onResponse interceptor.
      // Explicitly handling it here via `defer` + `callback->exception(...)`
      // results in double invocation.
      .deferValue([this,
                   protocol,
                   ctx,
                   request = std::move(serializedRequest),
                   kind,
                   callback](auto&&) mutable {
        return handlePythonServerCallback(
            protocol, ctx, std::move(request), kind, callback);
      });
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
  auto get_throw_wrapped = [](protocol::PROTOCOL_TYPES protocol) {
    return protocol ==
            apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
        ? &detail::throw_wrapped<
              apache::thrift::BinaryProtocolReader,
              apache::thrift::BinaryProtocolWriter>
        : &detail::throw_wrapped<
              apache::thrift::CompactProtocolReader,
              apache::thrift::CompactProtocolWriter>;
  };

  switch (kind) {
    case apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE: {
      auto callback = apache::thrift::HandlerCallbackBase::Ptr::make(
          std::move(req),
          std::move(ctxStack),
          apache::thrift::HandlerCallbackBase::MethodNameInfo{
              .serviceName = serviceName,
              .definingServiceName = serviceName,
              .methodName = methodName,
              .qualifiedMethodName =
                  fmt::format("{}.{}", serviceName, methodName)},
          nullptr,
          eb,
          executor,
          ctx,
          nullptr,
          nullptr,
          requestData);
      return dispatchRequestOneway(
          protocol,
          ctx,
          std::move(serializedRequest),
          kind,
          std::move(callback));
    }
    case apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE: {
      auto return_streaming = protocol ==
              apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
          ? &detail::return_streaming<
                apache::thrift::BinaryProtocolReader,
                apache::thrift::BinaryProtocolWriter>
          : &detail::return_streaming<
                apache::thrift::CompactProtocolReader,
                apache::thrift::CompactProtocolWriter>;
      auto callback = apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndServerStream<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr::
          make(
              std::move(req),
              std::move(ctxStack),
              apache::thrift::HandlerCallbackBase::MethodNameInfo{
                  .serviceName = serviceName,
                  .definingServiceName = serviceName,
                  .methodName = methodName,
                  .qualifiedMethodName =
                      fmt::format("{}.{}", serviceName, methodName)},
              return_streaming,
              get_throw_wrapped(protocol),
              ctx->getProtoSeqId(),
              eb,
              executor,
              ctx,
              nullptr,
              nullptr,
              requestData);
      return dispatchRequestStreaming(
          protocol,
          ctx,
          std::move(serializedRequest),
          kind,
          std::move(callback));
    }
    case apache::thrift::RpcKind::SINK: {
      auto return_sink = protocol ==
              apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
          ? &detail::return_sink<
                apache::thrift::BinaryProtocolReader,
                apache::thrift::BinaryProtocolWriter>
          : &detail::return_sink<
                apache::thrift::CompactProtocolReader,
                apache::thrift::CompactProtocolWriter>;
      auto callback = apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndSinkConsumer<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr::
          make(
              std::move(req),
              std::move(ctxStack),
              apache::thrift::HandlerCallbackBase::MethodNameInfo{
                  .serviceName = serviceName,
                  .definingServiceName = serviceName,
                  .methodName = methodName,
                  .qualifiedMethodName =
                      fmt::format("{}.{}", serviceName, methodName)},
              return_sink,
              get_throw_wrapped(protocol),
              ctx->getProtoSeqId(),
              eb,
              executor,
              ctx,
              nullptr,
              nullptr,
              requestData);
      return dispatchRequestSink(
          protocol,
          ctx,
          std::move(serializedRequest),
          kind,
          std::move(callback));
    }
    case apache::thrift::RpcKind::BIDIRECTIONAL_STREAM: {
      auto return_bidistream = protocol ==
              apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
          ? &detail::return_bidistream<
                apache::thrift::BinaryProtocolReader,
                apache::thrift::BinaryProtocolWriter>
          : &detail::return_bidistream<
                apache::thrift::CompactProtocolReader,
                apache::thrift::CompactProtocolWriter>;
      auto callback = apache::thrift::HandlerCallback<
          ::apache::thrift::ResponseAndStreamTransformation<
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>,
              std::unique_ptr<::folly::IOBuf>>>::Ptr::
          make(
              std::move(req),
              std::move(ctxStack),
              apache::thrift::HandlerCallbackBase::MethodNameInfo{
                  .serviceName = serviceName,
                  .definingServiceName = serviceName,
                  .methodName = methodName,
                  .qualifiedMethodName =
                      fmt::format("{}.{}", serviceName, methodName)},
              return_bidistream,
              get_throw_wrapped(protocol),
              ctx->getProtoSeqId(),
              eb,
              executor,
              ctx,
              nullptr,
              nullptr,
              requestData);
      return dispatchRequestBidi(
          protocol,
          ctx,
          std::move(serializedRequest),
          kind,
          std::move(callback));
    }
    default: {
      auto return_serialized = protocol ==
              apache::thrift::protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL
          ? &detail::return_serialized<
                apache::thrift::BinaryProtocolReader,
                apache::thrift::BinaryProtocolWriter>
          : &detail::return_serialized<
                apache::thrift::CompactProtocolReader,
                apache::thrift::CompactProtocolWriter>;
      auto callback = apache::thrift::
          HandlerCallback<std::unique_ptr<::folly::IOBuf>>::Ptr::make(
              std::move(req),
              std::move(ctxStack),
              apache::thrift::HandlerCallbackBase::MethodNameInfo{
                  .serviceName = serviceName,
                  .definingServiceName = serviceName,
                  .methodName = methodName,
                  .qualifiedMethodName =
                      fmt::format("{}.{}", serviceName, methodName)},
              return_serialized,
              get_throw_wrapped(protocol),
              ctx->getProtoSeqId(),
              eb,
              executor,
              ctx,
              nullptr,
              nullptr,
              requestData);
      return dispatchRequestResponse(
          protocol,
          ctx,
          std::move(serializedRequest),
          kind,
          std::move(callback));
    }
  }
}

} // namespace apache::thrift::python
