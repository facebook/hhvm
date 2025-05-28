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

#include <thrift/lib/cpp2/server/ServerModule.h>

namespace apache::thrift::python {

class PythonServerModule : public apache::thrift::ServerModule {
 public:
  explicit PythonServerModule(std::string name) : name_(std::move(name)) {}

  std::string getName() const override { return name_; }

  std::vector<std::shared_ptr<apache::thrift::ServiceInterceptorBase>>
  getServiceInterceptors() override {
    return interceptors_;
  }

  void addServiceInterceptor(
      std::shared_ptr<apache::thrift::ServiceInterceptorBase> interceptor) {
    interceptors_.push_back(std::move(interceptor));
  }

 private:
  std::string name_;
  std::vector<std::shared_ptr<apache::thrift::ServiceInterceptorBase>>
      interceptors_;
};
} // namespace apache::thrift::python
