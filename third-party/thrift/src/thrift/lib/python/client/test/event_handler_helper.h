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

#include <thrift/lib/cpp/EventHandlerBase.h>

namespace thrift::python::test {

using namespace apache::thrift;
class TestEventHandler : public TProcessorEventHandler {
 public:
  void postWrite(
      void* /*ctx*/, const char* /*fn_name*/, uint32_t /*bytes*/) override {
    throw std::runtime_error("from postWrite");
  }
};

std::shared_ptr<TProcessorEventHandler>& clientHandler() {
  static std::shared_ptr<TProcessorEventHandler> clientHandler =
      std::make_shared<TestEventHandler>();
  return clientHandler;
}

void addHandler() {
  TClientBase::addClientEventHandler(clientHandler());
}

void removeHandler() {
  TClientBase::removeClientEventHandler(clientHandler());
}

} // namespace thrift::python::test
