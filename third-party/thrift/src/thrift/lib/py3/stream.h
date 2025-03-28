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

#include <exception>
#include <memory>
#include <optional>

#include <Python.h>

#include <folly/Portability.h>
#include <folly/ScopeGuard.h>
#include <folly/futures/Future.h>
#include <folly/python/AsyncioExecutor.h>
#include <folly/python/async_generator.h>
#include <folly/python/executor.h>
#include <thrift/lib/cpp2/async/ClientBufferedStream.h>
#include <thrift/lib/cpp2/async/ServerStream.h>

namespace thrift::py3 {

void cancelPythonIterator(PyObject*);

#if FOLLY_HAS_COROUTINES

template <typename T>
class ClientBufferedStreamWrapper {
 public:
  ClientBufferedStreamWrapper() = default;
  explicit ClientBufferedStreamWrapper(
      apache::thrift::ClientBufferedStream<T>& s)
      : gen_{std::move(s).toAsyncGenerator()} {}

  folly::coro::Task<folly::Optional<T>> getNext() {
    auto res = co_await gen_.getNext();
    if (res.has_value()) {
      co_return std::move(res.value());
    }
    co_return folly::none;
  }

 private:
  folly::python::AsyncGeneratorWrapper<T&&> gen_;
};

template <typename Response, typename StreamElement>
apache::thrift::ResponseAndServerStream<Response, StreamElement>
createResponseAndServerStream(
    Response response, apache::thrift::ServerStream<StreamElement> stream) {
  return {std::move(response), std::move(stream)};
}

inline folly::Function<void()> pythonFuncToCppFunc(PyObject* func) {
  Py_INCREF(func);
  return [func = std::move(func)] {
    if (func != Py_None) {
      PyObject_CallObject(func, nullptr);
    }
    Py_DECREF(func);
  };
}

template <typename StreamElement>
apache::thrift::ServerStream<StreamElement> createAsyncIteratorFromPyIterator(
    PyObject* iter,
    folly::Executor* executor,
    folly::Function<void(
        PyObject*, folly::Promise<std::optional<StreamElement>>)> genNext) {
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
           genNext)]() mutable -> folly::coro::AsyncGenerator<StreamElement&&> {
        Py_INCREF(iter);
        auto innerGuard = folly::makeGuard([iter] { Py_DECREF(iter); });

        folly::CancellationCallback cb{
            co_await folly::coro::co_current_cancellation_token,
            [iter, executor, guard = std::move(innerGuard)]() mutable {
              folly::via(executor, [iter, guard = std::move(guard)] {
                cancelPythonIterator(iter);
              });
            }};

        while (true) {
          auto [madePromise, future] =
              folly::makePromiseContract<std::optional<StreamElement>>(
                  executor);
          folly::via(
              executor,
              [&genNext, iter, promise = std::move(madePromise)]() mutable {
                genNext(iter, std::move(promise));
              });
          auto val = co_await std::move(future);
          if (!val) {
            break;
          }
          co_yield std::move(val.value());
        }
      });
}

#else /* !FOLLY_HAS_COROUTINES */
#error  Thrift stream type support needs C++ coroutines, which are not currently available. \
        Use a modern compiler and pass appropriate options to enable C++ coroutine support, \
        or consider passing the Thrift compiler the mstch_py3:no_stream option in order to \
        ignore stream type fields when generating code.
#endif

} // namespace thrift::py3
