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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/test/gen-cpp2/UnimplementedMethod.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

namespace apache::thrift::test {

TEST(UnimplementedMethodTest, Basic) {
  auto service =
      std::make_shared<apache::thrift::ServiceHandler<UnimplementedMethod>>();
  ScopedServerInterfaceThread runner{service};
  auto client = runner.newClient<UnimplementedMethodAsyncClient>();

  try {
    client->semifuture_tm().get();
    ADD_FAILURE() << "Expected exception to be thrown, but none was";
  } catch (const TApplicationException& ex) {
    EXPECT_EQ(ex.getType(), TApplicationException::UNKNOWN_METHOD);
  } catch (...) {
    ADD_FAILURE() << "Caught unknown exception type";
  }

  try {
    client->semifuture_eb().get();
    ADD_FAILURE() << "Expected exception to be thrown, but none was";
  } catch (const TApplicationException& ex) {
    EXPECT_EQ(ex.getType(), TApplicationException::UNKNOWN_METHOD);
  } catch (...) {
    ADD_FAILURE() << "Caught unknown exception type";
  }
}

} // namespace apache::thrift::test
