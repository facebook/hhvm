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
#include <folly/python/error.h>
#include <folly/python/import.h>
#include <thrift/lib/python/streaming/Sink.h>
#include <thrift/lib/python/streaming/sink_api.h> // @manual

namespace apache::thrift::python {

namespace {

void do_python_import() {
  static ::folly::python::import_cache_nocapture import(
      (::import_thrift__python__streaming__sink));
  if (!import()) {
    ::folly::python::handlePythonError(
        "import error: thrift.python.streaming.sink");
  }
}

#if FOLLY_HAS_COROUTINES

template <typename TChunk>
folly::coro::AsyncGenerator<TChunk&&> toAsyncGeneratorImpl(
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
       executor = executor,
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

folly::Function<folly::coro::Task<std::unique_ptr<folly::IOBuf>>(
    folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&>)>
makeSinkCallback(PyObject* sink_callback, folly::Executor* exec) {
  do_python_import();

  // Must create this here to hold a KeepAlive for executor
  // and to create a new reference for the sink_callback py function.
  // Otherwise, sink_callback will be gc'd before callback invoked.
  return
      [sink_callback, exec, sink_gen = IOBufSinkGenerator(exec, sink_callback)](
          folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&>
              agen) mutable
          -> folly::coro::Task<std::unique_ptr<folly::IOBuf>> {
        auto [promise, future] =
            folly::makePromiseContract<std::unique_ptr<folly::IOBuf>>();
        sink_gen.attach(std::move(agen));

        const int result = invoke_server_sink_callback(
            sink_callback, exec, std::move(sink_gen), std::move(promise));
        if (result == -1) {
          folly::python::handlePythonError(
              "PythonAsyncProccessor: makeSinkCallback");
        }

        co_return co_await std::move(future);
      };
}
#endif // FOLLY_HAS_COROUTINES

} // namespace

void cancelPythonGenerator(PyObject* iter) {
  do_python_import();
  cancelAsyncGenerator(iter);
}

#if FOLLY_HAS_COROUTINES

folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&> toAsyncGenerator(
    PyObject* iter,
    folly::Executor* executor,
    folly::Function<void(
        PyObject*,
        folly::Promise<std::optional<std::unique_ptr<folly::IOBuf>>>)>
        genNext) {
  return toAsyncGeneratorImpl<std::unique_ptr<folly::IOBuf>>(
      iter, executor, std::move(genNext));
}

IOBufSinkGenerator::IOBufSinkGenerator(
    folly::Executor* executor, PyObject* sink_callback)
    : executor_(folly::getKeepAliveToken(CHECK_NOTNULL(executor))),
      sink_callback_(sink_callback) {
  Py_INCREF(sink_callback);
}
void IOBufSinkGenerator::attach(
    folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&> gen) noexcept {
  gen_ = std::move(gen);
}

IOBufSinkGenerator::~IOBufSinkGenerator() {
  if (sink_callback_ == nullptr) {
    return;
  }
  executor_->add(
      [sink_callback = sink_callback_] { Py_DECREF(sink_callback); });
}

IOBufSinkGenerator& IOBufSinkGenerator::operator=(
    IOBufSinkGenerator&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  gen_ = std::move(other.gen_);
  executor_ = std::move(other.executor_);
  sink_callback_ = CHECK_NOTNULL(other.sink_callback_);
  // prevent other dtor from scheduling a decref on the same callback
  other.sink_callback_ = nullptr;
  return *this;
}

IOBufSinkGenerator::IOBufSinkGenerator(IOBufSinkGenerator&& other) noexcept {
  gen_ = std::move(other.gen_);
  executor_ = std::move(other.executor_);
  sink_callback_ = CHECK_NOTNULL(other.sink_callback_);
  // prevent other dtor from scheduling a decref on the same callback
  other.sink_callback_ = nullptr;
}

folly::coro::Task<std::unique_ptr<folly::IOBuf>> IOBufSinkGenerator::getNext() {
  auto res = co_await gen_.next();
  if (res) {
    co_return std::move(*res);
  } else {
    // nullptr signals generator is done
    co_return nullptr;
  }
}

#endif // FOLLY_HAS_COROUTINES

std::unique_ptr<IOBufSinkConsumer> makeIOBufSinkConsumer(
    PyObject* sink_callback, folly::Executor* exec) {
#if FOLLY_HAS_COROUTINES
  auto sink_gen = IOBufSinkGenerator(exec, sink_callback);
  CHECK(!PyErr_Occurred());
  return std::make_unique<IOBufSinkConsumer>(
      IOBufSinkConsumer{.consumer = makeSinkCallback(sink_callback, exec)});
#else
  std::terminate();
#endif // FOLLY_HAS_COROUTINES
}

} // namespace apache::thrift::python
