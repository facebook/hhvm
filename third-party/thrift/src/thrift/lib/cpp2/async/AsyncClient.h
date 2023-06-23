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

#include <thrift/lib/cpp/EventHandlerBase.h>
#include <thrift/lib/cpp2/async/HeaderChannel.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache {
namespace thrift {
class InteractionHandle;

class GeneratedAsyncClient : public TClientBase {
 public:
  using channel_ptr =
      std::unique_ptr<RequestChannel, folly::DelayedDestruction::Destructor>;

  struct Options {
   public:
    Options() {}

    Options& includeGlobalEventHandlers(bool include) {
      clientBaseOptions_.includeGlobalEventHandlers = include;
      return *this;
    }

    static Options zeroDependency() {
      return Options().includeGlobalEventHandlers(false);
    }

   private:
    TClientBase::Options clientBaseOptions_;

    friend class GeneratedAsyncClient;
  };

  GeneratedAsyncClient(std::shared_ptr<RequestChannel> channel);
  GeneratedAsyncClient(
      std::shared_ptr<RequestChannel> channel, Options options);

  virtual const char* getServiceName() const noexcept = 0;

  RequestChannel* getChannel() const noexcept { return channel_.get(); }

  std::shared_ptr<RequestChannel> getChannelShared() const noexcept {
    return channel_;
  }

  HeaderChannel* getHeaderChannel() const noexcept {
    return dynamic_cast<HeaderChannel*>(channel_.get());
  }

 protected:
  static void setInteraction(
      const InteractionHandle& handle, RpcOptions& rpcOptions);

  std::shared_ptr<RequestChannel> channel_;
};

class InteractionHandle : public GeneratedAsyncClient {
 public:
  InteractionHandle(
      std::shared_ptr<RequestChannel> channel, folly::StringPiece methodName);
  InteractionHandle(std::shared_ptr<RequestChannel> channel, InteractionId id);
  ~InteractionHandle() override;
  InteractionHandle(InteractionHandle&&) noexcept = default;
  InteractionHandle& operator=(InteractionHandle&&);

  const InteractionId& getInteractionId();

 protected:
  void setInteraction(RpcOptions& rpcOptions) const;

 private:
  void terminate();

 protected:
  InteractionId interactionId_;

  friend class GeneratedAsyncClient;
};

} // namespace thrift
} // namespace apache
