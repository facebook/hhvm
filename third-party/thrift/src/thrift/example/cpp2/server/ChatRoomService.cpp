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

#include <thrift/example/cpp2/server/ChatRoomService.h>

#include <time.h>

DEFINE_int32(max_messages_per_get, 1000, "Maximum number of messages to fetch");

namespace example {
namespace chatroom {

void ChatRoomServiceHandler::getMessages(
    GetMessagesResponse& resp, std::unique_ptr<GetMessagesRequest> req) {
  int64_t idx = 0;
  if (auto token = req->token()) {
    idx = *token->index();
  }

  int64_t i = idx;
  messageBuffer_.withWLock([&](auto& lockedMessage) {
    int size = lockedMessage.size();
    int count = 0;
    while (i < size && count < FLAGS_max_messages_per_get) {
      resp.messages_ref()->push_back(lockedMessage[i]);
      ++i;
      ++count;
    }
  });

  IndexToken token;
  *token.index() = i;
  *resp.token() = token;
}

void ChatRoomServiceHandler::sendMessage(
    std::unique_ptr<SendMessageRequest> req) {
  Message msg;
  *msg.message() = *req->message();
  *msg.sender() = *req->sender();

  *msg.timestamp() = (int64_t)time(nullptr);

  messageBuffer_.withWLock([&](auto& lockedMessage) { //
    lockedMessage.push_back(msg);
  });
}
} // namespace chatroom
} // namespace example
