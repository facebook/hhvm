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

#include <vector>
#include <folly/Synchronized.h>

#include <thrift/example/if/gen-cpp2/ChatRoomService.h>

namespace example::chatroom {

class ChatRoomServiceHandler
    : virtual public apache::thrift::ServiceHandler<ChatRoomService> {
 public:
  ChatRoomServiceHandler() = default;

  explicit ChatRoomServiceHandler(int64_t /*currentTime*/)
      : ChatRoomServiceHandler() {}

  explicit ChatRoomServiceHandler(std::function<int64_t()> /*timeFn*/)
      : ChatRoomServiceHandler() {}

  void getMessages(
      GetMessagesResponse& resp,
      std::unique_ptr<GetMessagesRequest> req) override;

  void sendMessage(std::unique_ptr<SendMessageRequest> req) override;

 private:
  folly::Synchronized<std::vector<Message>> messageBuffer_ =
      folly::Synchronized<std::vector<Message>>();
};
} // namespace example::chatroom
