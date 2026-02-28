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

#include <memory>
#include <Python.h>
#include <folly/coro/Task.h>
#include <thrift/lib/cpp2/server/ServiceInterceptor.h>

namespace apache::thrift::python {
class PythonServiceInterceptor
    : public apache::thrift::ServiceInterceptor<PyObject*, PyObject*> {
 public:
  PythonServiceInterceptor() = default;
  ~PythonServiceInterceptor() override {}
  // Disable copy and move to force boxing
  PythonServiceInterceptor(const PythonServiceInterceptor&) = delete;
  PythonServiceInterceptor& operator=(const PythonServiceInterceptor&) = delete;
  PythonServiceInterceptor(PythonServiceInterceptor&&) = delete;
  PythonServiceInterceptor& operator=(PythonServiceInterceptor&&) = delete;
};

class ObservableServiceInterceptor : public PythonServiceInterceptor {
 public:
  ObservableServiceInterceptor(PyObject* pyWrapper, std::string name)
      : pyWrapper_(pyWrapper), name_(std::move(name)) {
    Py_IncRef(pyWrapper);
  }
  ~ObservableServiceInterceptor() override { Py_DecRef(pyWrapper_); }
  // Disable copy and move to avoid incorrect refcounting
  ObservableServiceInterceptor(const ObservableServiceInterceptor&) = delete;
  ObservableServiceInterceptor& operator=(const ObservableServiceInterceptor&) =
      delete;
  ObservableServiceInterceptor(ObservableServiceInterceptor&&) = delete;
  ObservableServiceInterceptor& operator=(ObservableServiceInterceptor&&) =
      delete;

  std::string getName() const override;

  std::optional<PyObject*> onConnectionEstablished(ConnectionInfo) override;
  void onConnectionClosed(
      PyObject** /* ConnectionState */, ConnectionInfo) noexcept override;

  folly::coro::Task<std::optional<PyObject*>> onRequest(
      PyObject** /* ConnectionState* */,
      ServiceInterceptorBase::RequestInfo) override;
  folly::coro::Task<void> onResponse(
      PyObject** /* RequestState* */,
      PyObject** /* ConnectionState* */,
      ServiceInterceptorBase::ResponseInfo) override;

 private:
  PyObject* pyWrapper_;
  std::string name_;
};

} // namespace apache::thrift::python
