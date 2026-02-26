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

#include <thrift/lib/cpp2/async/AsyncClient.h>

#include <folly/Overload.h>

#include <thrift/lib/cpp2/runtime/Init.h>

#include <string_view>

namespace apache::thrift {
namespace {

static InteractionId createInteraction(
    RequestChannel& channel, std::string_view methodName) {
  DCHECK(
      !channel.getEventBase() || channel.getEventBase()->isInEventBaseThread());
  return channel.createInteraction(methodName);
}

GeneratedAsyncClient::InterceptorList extractInterceptors(
    GeneratedAsyncClient::InterceptorSpecification&& spec) {
  using Result = GeneratedAsyncClient::InterceptorList;
  return folly::variant_match(
      std::move(spec),
      [](GeneratedAsyncClient::UseGlobalInterceptors) -> Result {
        return apache::thrift::runtime::getGlobalClientInterceptors();
      },
      [](GeneratedAsyncClient::InterceptorList&& interceptors) -> Result {
        return std::move(interceptors);
      });
}

} // namespace

GeneratedAsyncClient::GeneratedAsyncClient(
    std::shared_ptr<RequestChannel> channel)
    : GeneratedAsyncClient(std::move(channel), Options()) {}

GeneratedAsyncClient::GeneratedAsyncClient(
    std::shared_ptr<RequestChannel> channel, Options options)
    : GeneratedAsyncClient(
          std::move(channel), UseGlobalInterceptors(), std::move(options)) {}

GeneratedAsyncClient::GeneratedAsyncClient(
    std::shared_ptr<RequestChannel> channel,
    InterceptorSpecification interceptors,
    Options options)
    : TClientBase(options.clientBaseOptions_),
      channel_(std::move(channel)),
      interceptors_(extractInterceptors(std::move(interceptors))),
      ioBufFactory_(std::move(options.ioBufFactory_)) {}

void GeneratedAsyncClient::setInteraction(
    const InteractionHandle& handle, RpcOptions& rpcOptions) {
  handle.setInteraction(rpcOptions);
}

InteractionHandle::InteractionHandle(
    std::shared_ptr<RequestChannel> channel,
    folly::StringPiece methodName,
    InterceptorList interceptors)
    : GeneratedAsyncClient(channel, std::move(interceptors)),
      interactionId_(createInteraction(*channel, methodName)) {
  DCHECK(interactionId_);
}

InteractionHandle::InteractionHandle(
    std::shared_ptr<RequestChannel> channel,
    InteractionId id,
    InterceptorList interceptors)
    : GeneratedAsyncClient(channel, std::move(interceptors)),
      interactionId_(std::move(id)) {
  DCHECK(interactionId_);
}

InteractionHandle::~InteractionHandle() {
  terminate();
}
InteractionHandle& InteractionHandle::operator=(InteractionHandle&& other) {
  if (this != &other) {
    terminate();
  }
  channel_ = std::move(other.channel_);
  interactionId_ = std::move(other.interactionId_);
  return *this;
}

void InteractionHandle::setInteraction(RpcOptions& rpcOptions) const {
  DCHECK(interactionId_);
  DCHECK(rpcOptions.getInteractionId() == 0);
  rpcOptions.setInteractionId(interactionId_);
}

const InteractionId& InteractionHandle::getInteractionId() {
  DCHECK(interactionId_);
  return interactionId_;
}

void InteractionHandle::terminate() {
  if (!channel_ || !interactionId_) {
    return;
  }
  auto* eb = channel_->getEventBase();
  if (eb) {
    folly::getKeepAliveToken(eb).add(
        [channel = channel_, id = std::move(interactionId_)](auto&&) mutable {
          channel->terminateInteraction(std::move(id));
        });
  } else {
    channel_->terminateInteraction(std::move(interactionId_));
  }
}

} // namespace apache::thrift
