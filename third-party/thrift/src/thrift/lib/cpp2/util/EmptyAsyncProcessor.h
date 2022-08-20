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

#include <thrift/lib/cpp2/async/AsyncProcessor.h>

namespace apache::thrift {

/**
 * An AsyncProcessorFactory implementation that handles no methods. This can be
 * useful for testing.
 */
class EmptyAsyncProcessorFactory : public AsyncProcessorFactory {
  std::unique_ptr<AsyncProcessor> getProcessor() override;

  CreateMethodMetadataResult createMethodMetadata() override {
    return MethodMetadataMap{/* empty */};
  }
  std::vector<ServiceHandlerBase*> getServiceHandlers() override { return {}; }
};

} // namespace apache::thrift
