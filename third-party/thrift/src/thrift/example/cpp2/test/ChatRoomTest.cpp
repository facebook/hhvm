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

#include <thrift/example/cpp2/server/ChatRoomService.h>
#include <thrift/example/if/gen-cpp2/ChatRoomService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using example::chatroom::ChatRoomServiceAsyncClient;
using example::chatroom::ChatRoomServiceHandler;
using example::chatroom::GetMessagesRequest;
using example::chatroom::GetMessagesResponse;
using example::chatroom::SendMessageRequest;

namespace apache::thrift {

class ChatRoomTest : public testing::Test {
 public:
  ChatRoomTest() {
    handler_ = std::make_shared<ChatRoomServiceHandler>();
    client_ = makeTestClient<ChatRoomServiceAsyncClient>(handler_);
  }

  std::shared_ptr<ChatRoomServiceAsyncClient> client_;

 private:
  std::shared_ptr<ChatRoomServiceHandler> handler_;
};

TEST_F(ChatRoomTest, Example) {
  // Send RPC to Server
  SendMessageRequest sendRequest;
  *sendRequest.message() = "This is an example!";
  *sendRequest.sender() = "UnitTest";
  client_->sync_sendMessage(sendRequest);

  // Send RPC to get Results
  GetMessagesRequest getRequest;
  GetMessagesResponse response;
  client_->sync_getMessages(response, getRequest);
  EXPECT_EQ(response.messages()->size(), 1);
  EXPECT_EQ(*response.messages()->front().message(), *sendRequest.message());
  EXPECT_EQ(*response.messages()->front().sender(), *sendRequest.sender());

  // Repeat
  client_->sync_sendMessage(sendRequest);
  client_->sync_getMessages(response, getRequest);
  EXPECT_EQ(response.messages()->size(), 2);
}

} // namespace apache::thrift
