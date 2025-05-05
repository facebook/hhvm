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

#include <string_view>
#include <thrift/lib/cpp/TProcessorEventHandler.h>

namespace thrift::python::test {

class TestClientEventHandler : public apache::thrift::TProcessorEventHandler {
 public:
  void preWrite(void* ctx, std::string_view fn_name) override {
    preWriteCalled_ = true;
  }

  bool preWriteCalled() const { return preWriteCalled_; }

 private:
  bool preWriteCalled_ = false;
};
} // namespace thrift::python::test
