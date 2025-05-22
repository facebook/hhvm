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
#include <glog/logging.h>
#include <folly/futures/Future.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/gen/service_tcc.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/python/server/PythonAsyncProcessor.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift::python::test {

folly::SemiFuture<std::unique_ptr<folly::IOBuf>> get_serialized_metadata(
    std::shared_ptr<AsyncProcessorFactory> factory) {
  return folly::makeSemiFuture().deferValue([factory](auto) {
    std::unique_ptr<PythonAsyncProcessor> processor =
        std::unique_ptr<PythonAsyncProcessor>(
            dynamic_cast<PythonAsyncProcessor*>(
                factory->getProcessor().release()));

    return processor->getPythonMetadata();
  });
}

} // namespace apache::thrift::python::test
