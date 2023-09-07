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

#include <thrift/lib/cpp2/async/PreprocessingAsyncProcessorWrapper.h>

namespace apache {
namespace thrift {
PreprocessingAsyncProcessorWrapper::PreprocessingAsyncProcessorWrapper(
    std::unique_ptr<AsyncProcessor> innerProcessor)
    : innerProcessor_(std::move(innerProcessor)) {}

void PreprocessingAsyncProcessorWrapper::addEventHandler(
    const std::shared_ptr<TProcessorEventHandler>& handler) {
  auto* innerProcessor = inner();
  CHECK(innerProcessor != nullptr);
  innerProcessor->addEventHandler(handler);
}

AsyncProcessor* PreprocessingAsyncProcessorWrapper::inner() const noexcept {
  return innerProcessor_.get();
}

void PreprocessingAsyncProcessorWrapper::
    processSerializedCompressedRequestWithMetadata(
        ResponseChannelRequest::UniquePtr req,
        SerializedCompressedRequest&& serializedRequest,
        const MethodMetadata& methodMetadata,
        protocol::PROTOCOL_TYPES prot_type,
        Cpp2RequestContext* context,
        folly::EventBase* eb,
        concurrency::ThreadManager* tm) {
  auto [processedReq, processedSerializedReq] =
      processSerializedCompressedRequestWithMetadataImpl(
          std::move(req),
          std::move(serializedRequest),
          methodMetadata,
          prot_type,
          context,
          eb,
          tm);
  auto* innerProcessor = inner();
  CHECK(innerProcessor != nullptr);
  innerProcessor->processSerializedCompressedRequestWithMetadata(
      std::move(processedReq),
      std::move(processedSerializedReq),
      methodMetadata,
      prot_type,
      context,
      eb,
      tm);
}

const char* PreprocessingAsyncProcessorWrapper::getServiceName() {
  auto* innerProcessor = inner();
  CHECK(innerProcessor != nullptr);
  return innerProcessor->getServiceName();
}

void PreprocessingAsyncProcessorWrapper::executeRequest(
    ServerRequest&& request,
    const AsyncProcessorFactory::MethodMetadata& methodMetadata) {
  auto processedReq = executeRequestImpl(std::move(request), methodMetadata);
  auto* innerProcessor = inner();
  CHECK(innerProcessor != nullptr);
  innerProcessor->executeRequest(std::move(processedReq), methodMetadata);
}
} // namespace thrift
} // namespace apache
