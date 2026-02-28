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

#include <gtest/gtest.h>

#include <thrift/example/cpp2/server/EchoService.h>
#include <thrift/example/if/gen-cpp2/Echo.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using example::chatroom::EchoAsyncClient;
using example::chatroom::EchoHandler;

namespace apache::thrift {

class EchoTest : public testing::Test {
 public:
  EchoTest() {
    handler_ = std::make_shared<EchoHandler>();
    client_ = makeTestClient<EchoAsyncClient>(handler_);
  }

  std::shared_ptr<EchoAsyncClient> client_;

 private:
  std::shared_ptr<EchoHandler> handler_;
};

TEST_F(EchoTest, Example) {
  std::string echo = "Echo Message", response;
  client_->sync_echo(response, echo);
  EXPECT_EQ(echo, response);
}

} // namespace apache::thrift
