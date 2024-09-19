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

namespace apache::thrift {

class InteractionHandle;
class ClientInterceptorBase;

class GeneratedAsyncClient : public TClientBase {
 public:
  using channel_ptr =
      std::unique_ptr<RequestChannel, folly::DelayedDestruction::Destructor>;

  struct Options {
   public:
    Options() {}

    Options& includeGlobalLegacyEventHandlers(bool include) {
      clientBaseOptions_.includeGlobalLegacyClientHandlers = include;
      return *this;
    }

    static Options zeroDependency() {
      return Options().includeGlobalLegacyEventHandlers(false);
    }

   private:
    TClientBase::Options clientBaseOptions_;

    friend class GeneratedAsyncClient;
  };

  GeneratedAsyncClient(std::shared_ptr<RequestChannel> channel);
  GeneratedAsyncClient(
      std::shared_ptr<RequestChannel> channel, Options options);
  GeneratedAsyncClient(
      std::shared_ptr<RequestChannel> channel,
      std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>
          interceptors,
      Options options = Options());

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

  template <bool IsOneWay>
  std::pair<RequestClientCallback::Ptr, ContextStack*>
  prepareRequestClientCallback(
      std::unique_ptr<RequestCallback> callback,
      ContextStack::UniquePtr&& contextStack) {
    RequestCallback::Context callbackContext;
    callbackContext.oneWay = IsOneWay;
    callbackContext.protocolId = this->getChannel()->getProtocolId();
    auto* ctx = contextStack.get();
    if (callback) {
      callbackContext.ctx = std::move(contextStack);
    }
    auto wrappedCallback = apache::thrift::toRequestClientCallbackPtr(
        std::move(callback), std::move(callbackContext));
    return std::make_pair(std::move(wrappedCallback), ctx);
  }

  std::shared_ptr<RequestChannel> channel_;
  std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>
      interceptors_;
};

class InteractionHandle : public GeneratedAsyncClient {
 public:
  InteractionHandle(
      std::shared_ptr<RequestChannel> channel,
      folly::StringPiece methodName,
      std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>
          interceptors);
  InteractionHandle(
      std::shared_ptr<RequestChannel> channel,
      InteractionId id,
      std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>
          interceptors);
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

} // namespace apache::thrift
