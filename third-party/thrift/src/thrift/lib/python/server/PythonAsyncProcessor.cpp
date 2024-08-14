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

void PythonAsyncProcessor::processSerializedCompressedRequestWithMetadata(
    apache::thrift::ResponseChannelRequest::UniquePtr req,
    apache::thrift::SerializedCompressedRequest&& serializedRequest,
    const apache::thrift::AsyncProcessorFactory::MethodMetadata&
        untypedMethodMetadata,
    apache::thrift::protocol::PROTOCOL_TYPES protType,
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
  auto executor = tm
      ? tm->getKeepAlive(
            context->getRequestExecutionScope(),
            apache::thrift::concurrency::ThreadManager::Source::INTERNAL)
      : nullptr;
  (this->*pfn)(
      std::move(req), std::move(serializedRequest), context, eb, executor);
}

} // namespace python
} // namespace thrift
