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

#include <folly/Portability.h>
#include <folly/futures/Promise.h>
#include <thrift/lib/python/streaming/Sink.h>
#include <thrift/lib/python/streaming/bidistream.h>

namespace apache::thrift::python {

std::unique_ptr<apache::thrift::StreamTransformation<
    std::unique_ptr<folly::IOBuf>,
    std::unique_ptr<folly::IOBuf>>>
createIOBufStreamTransformation(PyObject* /*bidi*/, folly::Executor* /*exec*/) {
#if FOLLY_HAS_COROUTINES
  using TransformType = apache::thrift::StreamTransformation<
      std::unique_ptr<folly::IOBuf>,
      std::unique_ptr<folly::IOBuf>>;
  // TODO: Implement creating IOBuf transformation
  return std::make_unique<TransformType>();
#else
  std::terminate();
#endif // FOLLY_HAS_COROUTINES
}

} // namespace apache::thrift::python
