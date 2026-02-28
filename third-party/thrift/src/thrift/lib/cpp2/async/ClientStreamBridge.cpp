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

#include <thrift/lib/cpp2/async/ClientStreamBridge.h>

#include <folly/Overload.h>

namespace apache::thrift::detail {

// Explicitly instantiate the base of ClientStreamBridge
template class TwoWayBridge<
    QueueConsumer,
    ClientStreamMessageServerToClient,
    ClientStreamBridge,
    ClientStreamMessageClientToServer,
    ClientStreamBridge>;

ClientStreamBridge::ClientStreamBridge(FirstResponseCallback* callback)
    : firstResponseCallback_(callback) {}

ClientStreamBridge::~ClientStreamBridge() {}

void ClientStreamBridge::ClientDeleter::operator()(ClientStreamBridge* ptr) {
  ptr->cancel();
  Deleter::operator()(ptr);
}

StreamClientCallback* ClientStreamBridge::create(
    FirstResponseCallback* callback) {
  return new ClientStreamBridge(callback);
}

bool ClientStreamBridge::wait(QueueConsumer* consumer) {
  return clientWait(consumer);
}

ClientStreamBridge::ClientQueue ClientStreamBridge::getMessages() {
  return clientGetMessages();
}

void ClientStreamBridge::requestN(int64_t credits) {
  clientPush(StreamMessage::RequestN{static_cast<int32_t>(credits)});
}

void ClientStreamBridge::cancel() {
  clientPush(StreamMessage::Cancel{});
  clientClose();
}

bool ClientStreamBridge::isCanceled() {
  return isClientClosed();
}

void ClientStreamBridge::consume() {
  DCHECK(serverExecutor_);
  serverExecutor_->add([this]() { processCredits(); });
}

void ClientStreamBridge::canceled() {
  serverCleanup();
}

bool ClientStreamBridge::onFirstResponse(
    FirstResponsePayload&& payload,
    folly::EventBase* evb,
    StreamServerCallback* streamServerCallback) {
  auto firstResponseCallback = firstResponseCallback_;
  serverExecutor_ = evb;
  streamServerCallback_ = streamServerCallback;
  auto scheduledWait = serverWait(this);
  DCHECK(scheduledWait);
  firstResponseCallback->onFirstResponse(
      std::move(payload), ClientPtr(copy().release()));
  return true;
}

void ClientStreamBridge::onFirstResponseError(folly::exception_wrapper ew) {
  firstResponseCallback_->onFirstResponseError(std::move(ew));
  serverCleanup();
}

bool ClientStreamBridge::onStreamNext(StreamPayload&& payload) {
  serverPush(
      StreamMessage::PayloadOrError{
          folly::Try<StreamPayload>(std::move(payload))});
  return true;
}

void ClientStreamBridge::onStreamError(folly::exception_wrapper ew) {
  serverPush(
      StreamMessage::PayloadOrError{folly::Try<StreamPayload>(std::move(ew))});
  serverClose();
}

void ClientStreamBridge::onStreamComplete() {
  serverPush(StreamMessage::Complete{});
  serverClose();
}

bool ClientStreamBridge::onStreamHeaders(HeadersPayload&& payload) {
  serverPush(
      StreamMessage::PayloadOrError{
          folly::Try<StreamPayload>(StreamPayload(std::move(payload)))});
  return true;
}

void ClientStreamBridge::resetServerCallback(
    StreamServerCallback& serverCallback) {
  streamServerCallback_ = &serverCallback;
}

void ClientStreamBridge::processCredits() {
  if (isServerClosed()) {
    serverCleanup();
    return;
  }

  // serverClose() can't be called until this loop finishes
  int64_t credits = 0;
  while (!serverWait(this)) {
    for (auto messages = serverGetMessages(); !messages.empty();
         messages.pop()) {
      auto& message = messages.front();
      bool cancelled = folly::variant_match(
          message,
          [&](StreamMessage::RequestN requestN) {
            credits += requestN.n;
            return false;
          },
          [&](StreamMessage::Cancel) { return true; });
      if (cancelled) {
        streamServerCallback_->onStreamCancel();
        serverCleanup();
        return;
      }
    }
  }

  std::ignore = streamServerCallback_->onStreamRequestN(credits);
}

void ClientStreamBridge::serverCleanup() {
  streamServerCallback_ = nullptr;
  serverExecutor_.reset();
  Ptr(this);
}

} // namespace apache::thrift::detail
