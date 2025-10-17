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

#include <stdexcept>

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

#if FOLLY_HAS_COROUTINES

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
  auto output_py_gen = co_await std::move(future);

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

  // Capture bidi and exec for use in the transformation
  return
      [bidi, exec](folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&>
                       input_agen)
          -> folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&> {
        return transformAsyncGeneratorImpl(std::move(input_agen), bidi, exec);
      };
}

#endif // FOLLY_HAS_COROUTINES

} // namespace

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
