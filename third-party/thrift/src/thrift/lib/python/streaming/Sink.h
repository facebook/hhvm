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
#include <thrift/lib/cpp2/async/Sink.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift::python {

using IOBufClientSink = apache::thrift::
    ClientSink<std::unique_ptr<folly::IOBuf>, std::unique_ptr<folly::IOBuf>>;

void cancelPythonGenerator(PyObject*);

folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&> toAsyncGenerator(
    PyObject* iter,
    folly::Executor* executor,
    folly::Function<void(
        PyObject*,
        folly::Promise<std::optional<std::unique_ptr<folly::IOBuf>>>)> genNext);

#if FOLLY_HAS_COROUTINES
template <typename InitResponse, typename TChunk, typename FinalResponse>
apache::thrift::ResponseAndSinkConsumer<InitResponse, TChunk, FinalResponse>
createResponseAndSinkConsumer(
    InitResponse response,
    apache::thrift::SinkConsumer<TChunk, FinalResponse> sink) {
  return {std::move(response), std::move(sink)};
}

class IOBufSinkGenerator {
 public:
  IOBufSinkGenerator() = default;
  ~IOBufSinkGenerator();
  IOBufSinkGenerator(IOBufSinkGenerator&&) noexcept;
  IOBufSinkGenerator& operator=(IOBufSinkGenerator&&) noexcept;
  IOBufSinkGenerator(const IOBufSinkGenerator&) = delete;
  IOBufSinkGenerator& operator=(const IOBufSinkGenerator&) = delete;

  IOBufSinkGenerator(folly::Executor* exec, PyObject* sink_callback);

  void attach(
      folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&>
          gen) noexcept;

  folly::coro::Task<std::unique_ptr<folly::IOBuf>> getNext();

 private:
  folly::Executor::KeepAlive<> executor_;
  PyObject* sink_callback_ = nullptr;
  folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&> gen_;
};

#endif

using IOBufSinkConsumer =
    SinkConsumer<std::unique_ptr<folly::IOBuf>, std::unique_ptr<folly::IOBuf>>;

std::unique_ptr<IOBufSinkConsumer> makeIOBufSinkConsumer(
    PyObject* sink_callback, folly::Executor* exec);

} // namespace apache::thrift::python

#else /* !FOLLY_HAS_COROUTINES */
#error  Thrift sink type support needs C++ coroutines, which are not currently available. \
        Use a modern compiler and pass appropriate options to enable C++ coroutine support.
#endif
