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

#include <thrift/lib/python/client/py_bridge/PySender.h>

#include <stdexcept>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/futures/Promise.h>
#include <folly/python/import.h>

#include <thrift/lib/python/client/py_bridge/PyBridgeRequestChannel.h>
// Cython-generated API for the cdef-api `bridge_send_to_python`.
#include <thrift/lib/python/client/py_bridge/py_bridge_channel_api.h> // @manual

namespace apache::thrift::python::client {

namespace {

// Owns a counted reference to a Python `ChannelHandler`; refcount changes
// happen under the GIL. Move-only so it can live inside a folly::Function.
class PyHandlerRef {
 public:
  explicit PyHandlerRef(PyObject* handler) : handler_{handler} {
    PyGILState_STATE g = PyGILState_Ensure();
    Py_XINCREF(handler_);
    PyGILState_Release(g);
  }
  PyHandlerRef(PyHandlerRef&& other) noexcept : handler_{other.handler_} {
    other.handler_ = nullptr;
  }
  PyHandlerRef& operator=(PyHandlerRef&&) = delete;
  PyHandlerRef(const PyHandlerRef&) = delete;
  PyHandlerRef& operator=(const PyHandlerRef&) = delete;
  ~PyHandlerRef() {
    // Skip the decref if the channel outlives the interpreter (post
    // Py_Finalize): acquiring the GIL then would crash, and the refcount no
    // longer matters once the interpreter is gone. This check is best-effort:
    // it does not close the TOCTOU window where another thread begins
    // Py_Finalize between the check and PyGILState_Ensure(). The contract is
    // therefore that callers must drop their RequestChannel::Ptr (and thus this
    // channel) before initiating interpreter shutdown.
    if (handler_ == nullptr || Py_IsInitialized() == 0) {
      return;
    }
    PyGILState_STATE g = PyGILState_Ensure();
    Py_DECREF(handler_);
    PyGILState_Release(g);
  }

  folly::SemiFuture<std::unique_ptr<folly::IOBuf>> operator()(
      std::unique_ptr<folly::IOBuf> request, RpcKind rpcKind) {
    auto [promise, future] =
        folly::makePromiseContract<std::unique_ptr<folly::IOBuf>>();
    PyGILState_STATE g = PyGILState_Ensure();
    // Initialize the Cython module once before invoking its cdef-api symbol; a
    // failed import leaves the api table uninitialized, so we must not call
    // into it. Complete the promise with the error (after releasing the GIL)
    // rather than throwing under the GIL or leaving the caller's future
    // hanging.
    static ::folly::python::import_cache_nocapture import(
        (::import_thrift__python__client__py_bridge__py_bridge_channel));
    if (!import()) {
      PyGILState_Release(g);
      promise.setException(
          folly::make_exception_wrapper<std::runtime_error>(
              "import thrift.python.client.py_bridge.py_bridge_channel failed"));
      return std::move(future);
    }
    bridge_send_to_python(
        handler_,
        std::move(request),
        static_cast<int>(rpcKind),
        std::move(promise));
    PyGILState_Release(g);
    return std::move(future);
  }

 private:
  PyObject* handler_;
};

} // namespace

RequestChannel::Ptr makeBridgedChannel(
    PyObject* handler, uint16_t protocolId, folly::Executor* executor) {
  return PyBridgeRequestChannel::newChannel(
      protocolId, PyHandlerRef{handler}, executor);
}

} // namespace apache::thrift::python::client
