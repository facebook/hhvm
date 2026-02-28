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

#include <glog/logging.h>
#include <folly/Portability.h>
#include <folly/futures/Promise.h>
#include <folly/python/error.h>
#include <folly/python/import.h>
#include <thrift/lib/python/streaming/Sink.h>
#include <thrift/lib/python/streaming/bidistream.h>
#include <thrift/lib/python/streaming/bidistream_api.h> // @manual
#include <thrift/lib/python/streaming/py_promise_api.h> // @manual

namespace apache::thrift::python {

namespace {

bool do_import() {
  static ::folly::python::import_cache_nocapture bidi_import(
      (::import_thrift__python__streaming__bidistream));
  static ::folly::python::import_cache_nocapture py_promise_import(
      (::import_thrift__python__streaming__py_promise));
  return bidi_import() && py_promise_import();
}

} // namespace

#if FOLLY_HAS_COROUTINES

// BidiCallbackWrapper implementation
BidiCallbackWrapper::BidiCallbackWrapper(
    folly::Executor* executor, PyObject* bidi_callback)
    : executor_(folly::getKeepAliveToken(CHECK_NOTNULL(executor))),
      bidi_callback_(bidi_callback) {
  Py_INCREF(bidi_callback);
}

BidiCallbackWrapper::~BidiCallbackWrapper() {
  if (bidi_callback_ == nullptr) {
    return;
  }
  executor_->add(
      [bidi_callback = bidi_callback_] { Py_DECREF(bidi_callback); });
}

BidiCallbackWrapper::BidiCallbackWrapper(BidiCallbackWrapper&& other) noexcept {
  executor_ = std::move(other.executor_);
  bidi_callback_ = CHECK_NOTNULL(std::exchange(other.bidi_callback_, nullptr));
}

BidiCallbackWrapper& BidiCallbackWrapper::operator=(
    BidiCallbackWrapper&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  executor_ = std::move(other.executor_);
  bidi_callback_ = CHECK_NOTNULL(std::exchange(other.bidi_callback_, nullptr));
  return *this;
}

// Helper coroutine function implementing the transformation logic
folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&>
transformAsyncGeneratorImpl(
    folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&> input_agen,
    PyObject* bidi,
    folly::Executor* exec) {
  // Create IOBufSinkGenerator to wrap input generator for Python consumption
  auto input_gen = IOBufSinkGenerator(exec, bidi);
  input_gen.attach(std::move(input_agen));

  // Create promise/future pair for getting output generator from Python
  auto [promise, future] = folly::makePromiseContract<PyObject*>();

  // Call the Python bidi callback
  const int result = invoke_server_bidi_callback(
      bidi, exec, std::move(input_gen), std::move(promise));
  if (result == -1) {
    folly::python::handlePythonError("PythonAsyncProcessor: makeBidiCallback");
  }

  // Get the Python output generator from the promise
  // The promise transfers ownership of a reference to us
  // (Promise_PyObject.complete did Py_INCREF before setting the value), so we
  // need to Py_DECREF when done.
  auto output_py_gen = co_await std::move(future);

  // Create a guard to ensure we decref the output generator when this coroutine
  // completes. This balances the Py_INCREF done in Promise_PyObject.complete().
  // toAsyncGenerator will do its own Py_INCREF internally.
  auto output_gen_guard =
      folly::makeGuard([output_py_gen, exec = folly::getKeepAliveToken(exec)] {
        exec->add([output_py_gen] { Py_DECREF(output_py_gen); });
      });

  // Convert Python async generator back to C++ AsyncGenerator
  auto cpp_output_gen =
      toAsyncGenerator(output_py_gen, exec, genNextStreamValue);

  // Yield all elements from the output generator
  while (auto elem = co_await cpp_output_gen.next()) {
    co_yield std::move(*elem);
  }
}

folly::Function<folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&>(
    folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&>)>
makeTransformFunc(PyObject* bidi, folly::Executor* exec) {
  if (!do_import()) {
    ::folly::python::handlePythonError("Python module import error");
  }

  // Wrap bidi callback to manage reference counting
  auto bidi_wrapper = BidiCallbackWrapper(exec, bidi);

  // Capture wrapper for use in the transformation
  return [bidi_wrapper = std::move(bidi_wrapper)](
             folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&>
                 input_agen) mutable
             -> folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&> {
    return transformAsyncGeneratorImpl(
        std::move(input_agen), bidi_wrapper.get(), bidi_wrapper.getExecutor());
  };
}

#endif // FOLLY_HAS_COROUTINES

std::unique_ptr<apache::thrift::StreamTransformation<
    std::unique_ptr<folly::IOBuf>,
    std::unique_ptr<folly::IOBuf>>>
createIOBufStreamTransformation(PyObject* bidi, folly::Executor* exec) {
#if FOLLY_HAS_COROUTINES
  using TransformType = apache::thrift::StreamTransformation<
      std::unique_ptr<folly::IOBuf>,
      std::unique_ptr<folly::IOBuf>>;
  return std::make_unique<TransformType>(
      TransformType{.func = makeTransformFunc(bidi, exec)});
#else
  std::terminate();
#endif // FOLLY_HAS_COROUTINES
}

} // namespace apache::thrift::python
