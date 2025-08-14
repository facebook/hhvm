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

using IOBufClientSink =
    apache::thrift::ClientSink<std::unique_ptr<folly::IOBuf>, folly::IOBuf>;

void cancelPythonGenerator(PyObject*);

template <typename TChunk>
folly::coro::AsyncGenerator<TChunk&&> toAsyncGenerator(
    PyObject* iter,
    folly::Executor* executor,
    folly::Function<void(PyObject*, folly::Promise<std::optional<TChunk>>)>
        genNext) {
  Py_INCREF(iter);
  auto guard =
      folly::makeGuard([iter, executor = folly::getKeepAliveToken(executor)] {
        // Ensure the Python async generator is destroyed on a Python thread
        executor->add([iter] { Py_DECREF(iter); });
      });

  return folly::coro::co_invoke(
      [iter,
       executor = std::move(executor),
       guard = std::move(guard),
       genNext = std::move(
           genNext)]() mutable -> folly::coro::AsyncGenerator<TChunk&&> {
        Py_INCREF(iter);
        auto innerGuard = folly::makeGuard([iter] { Py_DECREF(iter); });

        folly::CancellationCallback cb{
            co_await folly::coro::co_current_cancellation_token,
            [iter, executor, guard = std::move(innerGuard)]() mutable {
              folly::via(executor, [iter, guard = std::move(guard)] {
                cancelPythonGenerator(iter);
              });
            }};

        while (true) {
          auto [promise, future] =
              folly::makePromiseContract<std::optional<TChunk>>(executor);
          folly::via(
              executor,
              [&genNext, iter, promise_ = std::move(promise)]() mutable {
                genNext(iter, std::move(promise_));
              });
          auto val = co_await std::move(future);
          if (!val) {
            break;
          }
          co_yield std::move(val.value());
        }
      });
}

#if FOLLY_HAS_COROUTINES
template <typename InitResponse, typename TChunk, typename FinalResponse>
apache::thrift::ResponseAndSinkConsumer<InitResponse, TChunk, FinalResponse>
createResponseAndSinkConsumer(
    InitResponse response,
    apache::thrift::SinkConsumer<TChunk, FinalResponse> sink) {
  return {std::move(response), std::move(sink)};
}

template <typename TChunk, typename FinalResponse>
folly::coro::Task<FinalResponse> sinkDummyCallback(
    folly::coro::AsyncGenerator<TChunk&&>);

template <>
inline folly::coro::Task<std::unique_ptr<folly::IOBuf>> sinkDummyCallback(
    folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&> agen) {
  std::string buf;
  while (auto chunk = co_await agen.next()) {
    (*chunk)->appendTo(buf);
  }
  co_return folly::IOBuf::fromString(std::move(buf));
}
#endif

using IOBufSinkConsumer =
    SinkConsumer<std::unique_ptr<folly::IOBuf>, std::unique_ptr<folly::IOBuf>>;

inline std::unique_ptr<IOBufSinkConsumer> makeIOBufSinkConsumer(
    PyObject* /*sink*/, folly::Executor*) {
#if FOLLY_HAS_COROUTINES
  return std::make_unique<IOBufSinkConsumer>(IOBufSinkConsumer{
      .consumer = sinkDummyCallback<
          std::unique_ptr<folly::IOBuf>,
          std::unique_ptr<folly::IOBuf>>});
#else
  std::terminate();
#endif
}

} // namespace apache::thrift::python

#else /* !FOLLY_HAS_COROUTINES */
#error  Thrift sink type support needs C++ coroutines, which are not currently available. \
        Use a modern compiler and pass appropriate options to enable C++ coroutine support, \
        or consider passing the Thrift compiler the mstch_python:no_sink (TODO: (pyamane) implement this) \
        option in order to ignore sink type fields when generating code.
#endif
