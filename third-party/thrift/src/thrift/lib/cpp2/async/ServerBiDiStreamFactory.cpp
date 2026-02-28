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

#include <thrift/lib/cpp2/async/ServerBiDiStreamFactory.h>

namespace apache::thrift::detail {

ServerBiDiStreamFactory::ServerBiDiStreamFactory() : startFunction_{nullptr} {}

/* static */ ServerBiDiStreamFactory
ServerBiDiStreamFactory::makeNoopFactory() {
  return ServerBiDiStreamFactory();
}

void ServerBiDiStreamFactory::setContextStack(
    std::shared_ptr<ContextStack> contextStack) {
  contextStack_ = std::move(contextStack);
}

void ServerBiDiStreamFactory::setInteraction(TilePtr&& interaction) {
  interaction_ = std::move(interaction);
}

bool ServerBiDiStreamFactory::valid() const {
  // TODO(ezou) this is a stub, revisit this.
  return startFunction_ != nullptr;
}

void ServerBiDiStreamFactory::start(
    FirstResponsePayload&& payload,
    BiDiClientCallback* clientCallback,
    folly::EventBase* eventBase) && {
  // startFunction_ == nullptr indicates that the ServerBiDiStreamFactory was
  // created with makeNoopFactory(), i.e the stream should do nothing. This
  // is done if the method that is supposed to initiate the stream throws
  // an exception.
  if (startFunction_ == nullptr) {
    return;
  }

  startFunction_(
      std::move(contextStack_),
      std::move(interaction_),
      std::move(payload),
      clientCallback,
      eventBase);
}

} // namespace apache::thrift::detail
