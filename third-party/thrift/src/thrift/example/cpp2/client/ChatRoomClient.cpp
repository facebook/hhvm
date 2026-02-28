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

#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <thrift/example/if/gen-cpp2/ChatRoomService.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>

DEFINE_string(host, "::1", "ChatRoomServer host");
DEFINE_int32(port, 7777, "ChatRoomServer port");

using example::chatroom::ChatRoomServiceAsyncClient;

int main(int argc, char* argv[]) {
  FLAGS_logtostderr = true;
  const folly::Init init(&argc, &argv);

  // Create an EventBase.
  folly::EventBase eventBase;

  // Create a Thrift client.
  auto socket = folly::AsyncSocket::UniquePtr(
      new folly::AsyncSocket(&eventBase, FLAGS_host, FLAGS_port));
  auto channel =
      apache::thrift::RocketClientChannel::newChannel(std::move(socket));
  auto client =
      std::make_unique<ChatRoomServiceAsyncClient>(std::move(channel));

  try {
    // Send a chat message via a Thrift request.
    auto sendRequest = example::chatroom::SendMessageRequest();
    *sendRequest.message() = "This is an example!";
    *sendRequest.sender() = getenv("USER");
    client->sync_sendMessage(sendRequest);

    // Get chat response messages via another Thrift request.
    auto getRequest = example::chatroom::GetMessagesRequest();
    auto response = example::chatroom::GetMessagesResponse();
    client->sync_getMessages(response, getRequest);

    // Print all the messages so far.
    for (auto& messagesList : *response.messages()) {
      LOG(INFO) << "Message: " << *messagesList.message()
                << " Sender: " << *messagesList.sender();
    }
  } catch (apache::thrift::transport::TTransportException& ex) {
    LOG(ERROR) << "Request failed " << ex.what();
  } catch (example::chatroom::Exception& ex) {
    LOG(ERROR) << "Request failed " << ex.what();
  }
}
