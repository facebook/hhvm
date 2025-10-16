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

#include <Python.h>
#include <folly/Executor.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/futures/Future.h>
#include <thrift/lib/cpp2/async/BiDiStream.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift::python {

template <typename TResponse, typename TSinkChunk, typename TStreamChunk>
apache::thrift::
    ResponseAndStreamTransformation<TResponse, TSinkChunk, TStreamChunk>
    createResponseAndStreamTransformation(
        TResponse response,
        apache::thrift::StreamTransformation<TSinkChunk, TStreamChunk> bidi) {
  return {std::move(response), std::move(bidi)};
}

using IOBufStreamTransformation = apache::thrift::StreamTransformation<
    std::unique_ptr<folly::IOBuf>,
    std::unique_ptr<folly::IOBuf>>;

std::unique_ptr<apache::thrift::StreamTransformation<
    std::unique_ptr<folly::IOBuf>,
    std::unique_ptr<folly::IOBuf>>>
createIOBufStreamTransformation(PyObject* bidi, folly::Executor* exec);

} // namespace apache::thrift::python

#else /* !FOLLY_HAS_COROUTINES */
#error  Thrift bi-directional type support needs C++ coroutines, which are not currently available. \
        Use a modern compiler and pass appropriate options to enable C++ coroutine support.
#endif
