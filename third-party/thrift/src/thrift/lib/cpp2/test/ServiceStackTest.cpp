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

#include <memory>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/test/gen-cpp2/TestServiceStack.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::test;

class ServiceStackHandler
    : public apache::thrift::ServiceHandler<TestServiceStack> {
 public:
  void noResponse(int64_t /* size */) override {}
};

TEST(ServiceStackTest, example) {
  auto handler = make_shared<ServiceStackHandler>();
  ScopedServerInterfaceThread ssit(handler);
  auto client = ssit.newClient<TestServiceStackAsyncClient>();
  client->sync_noResponse(/* p_size = */ 77);
}
