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
#include <memory>
#include <tuple>
#include <folly/CppAttributes.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>

namespace apache {
namespace thrift {

/**
 * PreprocessingAsyncProcessorWrapper should be considered whenever you need to
 * override AsyncProcessor to add additional logic aside (like profiling,
 * tracking resources etc.) from the usual request processing.
 *
 * Derived class should implement
 * processSerializedCompressedRequestWithMetadataImpl and executeRequestImpl
 * accordingly. These two functions are required to return the request struct.
 * If the asyncprocessor does not need to modify the default behavior, the
 * function may simple return passed in structs.
 *
 * Optionally, derived class may choose to override inner() function to control
 * whether delegated processor should be excuted or not after executing logic in
 * impl. If inner() function returns nullptr, the inner processor will not be
 * executed.
 */
class PreprocessingAsyncProcessorWrapper : public AsyncProcessor {
 public:
  explicit PreprocessingAsyncProcessorWrapper(
      std::unique_ptr<AsyncProcessor> innerProcessor);

  void addEventHandler(
      const std::shared_ptr<TProcessorEventHandler>& handler) override final;

  AsyncProcessor* FOLLY_NONNULL inner() const noexcept;

  void processSerializedCompressedRequestWithMetadata(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      const MethodMetadata& methodMetadata,
      protocol::PROTOCOL_TYPES prot_type,
      Cpp2RequestContext* context,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm) override final;

  void executeRequest(
      ServerRequest&& request,
      const AsyncProcessorFactory::MethodMetadata& methodMetadata)
      override final;

  const char* getServiceName() override final;

 protected:
  using ProcessSerializedCompressedRequestReturnT = std::
      tuple<ResponseChannelRequest::UniquePtr, SerializedCompressedRequest>;

  virtual ProcessSerializedCompressedRequestReturnT
  processSerializedCompressedRequestWithMetadataImpl(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      const MethodMetadata& methodMetadata,
      protocol::PROTOCOL_TYPES prot_type,
      Cpp2RequestContext* context,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm) = 0;

  virtual ServerRequest executeRequestImpl(
      ServerRequest&& request,
      const AsyncProcessorFactory::MethodMetadata& methodMetadata) = 0;

 private:
  std::unique_ptr<AsyncProcessor> innerProcessor_;
};

} // namespace thrift
} // namespace apache
