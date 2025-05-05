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
#include <string>
#include <thrift/lib/cpp/TProcessorEventHandler.h>

namespace thrift::python::test {

class ThriftPythonUnitTestException : public std::exception {
 public:
  explicit ThriftPythonUnitTestException(std::string&& unit_test)
      : unit_test_(std::move(unit_test)) {}
  const char* what() const noexcept override { return unit_test_.c_str(); }

 private:
  std::string unit_test_;
};

class ThrowPreReadClientEventHandler
    : public apache::thrift::TProcessorEventHandler {
 public:
  void preRead(void* /*ctx*/, std::string_view /*fn_name*/) override {
    throw ThriftPythonUnitTestException("pre_read");
  }
};

class ThrowOnReadClientEventHandler
    : public apache::thrift::TProcessorEventHandler {
 public:
  void onReadData(
      void* /*ctx*/,
      std::string_view /*fn_name*/,
      const apache::thrift::SerializedMessage& /*msg*/) override {
    throw ThriftPythonUnitTestException("on_read");
  }
};

class ThrowPostReadClientEventHandler
    : public apache::thrift::TProcessorEventHandler {
 public:
  void postRead(
      void* /*ctx*/,
      std::string_view /*fn_name*/,
      apache::thrift::transport::THeader* /*header*/,
      uint32_t /*bytes*/) override {
    throw ThriftPythonUnitTestException("post_read");
  }
};

} // namespace thrift::python::test
