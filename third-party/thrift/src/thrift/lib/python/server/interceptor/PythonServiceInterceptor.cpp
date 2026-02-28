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

#include <folly/python/error.h>
#include <folly/python/import.h>
#include <thrift/lib/python/server/interceptor/PythonServiceInterceptor.h>
#include <thrift/lib/python/server/interceptor/service_interceptor_api.h>

namespace apache::thrift::python {
namespace {
void ensure_module_imported() {
  static ::folly::python::import_cache_nocapture import(
      import_thrift__python__server_impl__interceptor__service_interceptor);
  if (!import()) {
    folly::python::handlePythonError(
        "Module import failed: thrift.python.server_impl.interceptor.service_interceptor");
  }
}

struct GILGuard {
  GILGuard() : gstate(::PyGILState_Ensure()) {}
  ~GILGuard() { ::PyGILState_Release(gstate); }

  GILGuard(const GILGuard&) = delete;
  GILGuard(GILGuard&&) = delete;
  GILGuard& operator=(const GILGuard&) = delete;
  GILGuard& operator=(GILGuard&&) = delete;

 private:
  ::PyGILState_STATE gstate;
};

PyObject* pyObjectOrNone(PyObject** maybeNullPyObject) {
  return maybeNullPyObject ? *maybeNullPyObject : Py_None;
}

PyObject* pyConnectionInfo(const ServiceInterceptorBase::ConnectionInfo& info) {
  return info.context ? make_connection_info(info.context) : Py_None;
}

PyObject* pyRequestInfo(const ServiceInterceptorBase::RequestInfo& info) {
  return info.context ? make_request_info(
                            info.context,
                            info.serviceName,
                            info.definingServiceName,
                            info.methodName)
                      : Py_None;
}

PyObject* pyResponseInfo(const ServiceInterceptorBase::ResponseInfo& info) {
  std::optional<std::string> errMessage;
  if (std::holds_alternative<folly::exception_wrapper>(
          info.resultOrActiveException)) {
    errMessage =
        std::get<folly::exception_wrapper>(info.resultOrActiveException).what();
  }
  return info.context ? make_response_info(
                            info.context,
                            info.serviceName,
                            info.definingServiceName,
                            info.methodName,
                            std::move(errMessage))
                      : Py_None;
}

} // namespace

std::string ObservableServiceInterceptor::getName() const {
  return name_;
}

std::optional<PyObject*> ObservableServiceInterceptor::onConnectionEstablished(
    ServiceInterceptorBase::ConnectionInfo connectionInfo) {
  GILGuard gil;
  ensure_module_imported();
  PyObject* pyConnInfo = pyConnectionInfo(connectionInfo);
  PyObject* result = call_on_connect_callback(pyWrapper_, pyConnInfo);
  if (!result) {
    // stringifies python error to std::runtime_error and throws
    folly::python::handlePythonError(
        "ObservableServiceInterceptor::onRequest Python callback error: ");
  }
  return result != Py_None ? std::make_optional(result) : std::nullopt;
}

void ObservableServiceInterceptor::onConnectionClosed(
    PyObject** connectionStatePtr,
    ServiceInterceptorBase::ConnectionInfo connectionInfo) noexcept {
  GILGuard gil;
  ensure_module_imported();
  PyObject* pyConnState = pyObjectOrNone(connectionStatePtr);
  PyObject* pyConnInfo = pyConnectionInfo(connectionInfo);
  PyObject* result =
      call_on_connect_closed_callback(pyWrapper_, pyConnState, pyConnInfo);
  if (!result) {
    // clear Python error and terminate
    try {
      folly::python::handlePythonError(
          "ObservableServiceInterceptor::onRequest Python callback error: ");
    } catch (const std::exception& e) {
      LOG(FATAL)
          << "Python error encountered in `noexcept` method `onConnectionClosed`: "
          << e.what();
    }
  }
}

folly::coro::Task<std::optional<PyObject*>>
ObservableServiceInterceptor::onRequest(
    PyObject** connectionStatePtr,
    ServiceInterceptorBase::RequestInfo requestInfo) {
  GILGuard gil;
  ensure_module_imported();
  PyObject* pyConnState = pyObjectOrNone(connectionStatePtr);
  PyObject* pyReqInfo = pyRequestInfo(requestInfo);
  PyObject* result =
      call_on_request_callback(pyWrapper_, pyConnState, pyReqInfo);
  if (!result) {
    // stringifies python error to std::runtime_error and throws
    folly::python::handlePythonError(
        "ObservableServiceInterceptor::onRequest Python callback error: ");
  }
  co_return result != Py_None ? std::make_optional(result) : std::nullopt;
}

folly::coro::Task<void> ObservableServiceInterceptor::onResponse(
    PyObject** requestStatePtr,
    PyObject** connectionStatePtr,
    ServiceInterceptorBase::ResponseInfo responseInfo) {
  GILGuard gil;
  ensure_module_imported();
  PyObject* pyReqState = pyObjectOrNone(requestStatePtr);
  PyObject* pyConnState = pyObjectOrNone(connectionStatePtr);
  PyObject* pyRespInfo = pyResponseInfo(responseInfo);

  PyObject* result = call_on_response_callback(
      pyWrapper_, pyReqState, pyConnState, pyRespInfo);
  if (!result) {
    // stringifies python error to std::runtime_error and throws
    folly::python::handlePythonError(
        "ObservableServiceInterceptor::onResponse Python callback error: ");
  }

  co_return;
}

} // namespace apache::thrift::python
