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

#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/python/test/gen-cpp2/TestingService.tcc>

namespace thrift::py3::test {

class TestingService
    : public apache::thrift::ServiceHandler<cpp2::TestingService> {
 public:
  static std::shared_ptr<TestingService> createInstance() {
    return std::make_shared<TestingService>();
  }

  void hard_error(bool valid) override {
    cpp2::HardError error;
    error.code() = 0;
    error.errortext() = valid ? "valid UTF-8" : "\xfa\xf0";
    throw error;
  }
};

} // namespace thrift::py3::test
