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

#include <folly/Portability.h>
#include <folly/python/error.h>
#include <thrift/lib/python/streaming/Sink.h>
#include <thrift/lib/python/streaming/sink_api.h> // @manual

namespace apache::thrift::python {

namespace {

void do_import() {
  if (0 != import_thrift__python__streaming__sink()) {
    throw std::runtime_error("import_thrift__python__sink__cancel failed");
  }
}

#if FOLLY_HAS_COROUTINES

folly::Function<folly::coro::Task<std::unique_ptr<folly::IOBuf>>(
    folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&>)>
makeSinkCallback(PyObject* sink_callback, folly::Executor* exec) {
  [[maybe_unused]] static bool done = (do_import(), false);

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
  [[maybe_unused]] static bool done = (do_import(), false);
  cancelAsyncGenerator(iter);
}

#if FOLLY_HAS_COROUTINES

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
  try {
    auto res = co_await gen_.next();
    if (res) {
      co_return std::move(*res);
    } else {
      // nullptr signals generator is done
      co_return nullptr;
    }
  } catch (...) {
    folly::exception_wrapper ew(std::current_exception());
    LOG(FATAL) << "Unexpected exception: " << ew.what();
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
