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

#include <thrift/lib/cpp2/util/EmptyAsyncProcessor.h>

namespace apache::thrift {

std::unique_ptr<AsyncProcessor> EmptyAsyncProcessorFactory::getProcessor() {
  class EmptyAsyncProcessor : public AsyncProcessor {
    void processSerializedCompressedRequestWithMetadata(
        ResponseChannelRequest::UniquePtr,
        SerializedCompressedRequest&&,
        const MethodMetadata&,
        protocol::PROTOCOL_TYPES,
        Cpp2RequestContext*,
        folly::EventBase*,
        concurrency::ThreadManager*) override {
      LOG(FATAL) << "This method should never be called on EmptyAsyncProcessor";
    }
    void processInteraction(ServerRequest&&) override {
      LOG(FATAL)
          << "This AsyncProcessor doesn't support Thrift interactions. "
          << "Please implement processInteraction to support interactions.";
    }
  };

  return std::make_unique<EmptyAsyncProcessor>();
}

} // namespace apache::thrift
