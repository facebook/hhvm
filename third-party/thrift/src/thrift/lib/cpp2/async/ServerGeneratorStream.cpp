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

#include <thrift/lib/cpp2/async/ServerGeneratorStream.h>

namespace apache {
namespace thrift {
namespace detail {

// Explicitly instantiate the base of ServerGeneratorStream
template class TwoWayBridge<
    ServerGeneratorStream,
    folly::Try<StreamPayload>,
    ServerStreamConsumer,
    int64_t,
    ServerGeneratorStream>;

ServerGeneratorStream::ServerGeneratorStream(
    StreamClientCallback* clientCallback, folly::EventBase* clientEb)
    : streamClientCallback_(clientCallback), clientEventBase_(clientEb) {}

ServerGeneratorStream::~ServerGeneratorStream() {}

void ServerGeneratorStream::consume() {
  clientEventBase_->add([this]() { processPayloads(); });
}
void ServerGeneratorStream::canceled() {
  Ptr(this);
}

bool ServerGeneratorStream::wait(ServerStreamConsumer* consumer) {
  return serverWait(consumer);
}

void ServerGeneratorStream::publish(folly::Try<StreamPayload>&& payload) {
  serverPush(std::move(payload));
}

ServerGeneratorStream::ServerQueue ServerGeneratorStream::getMessages() {
  return serverGetMessages();
}

bool ServerGeneratorStream::onStreamRequestN(uint64_t credits) {
  clientPush(std::move(credits));
  return true;
}

void ServerGeneratorStream::onStreamCancel() {
#if FOLLY_HAS_COROUTINES
  cancelSource_.requestCancellation();
#endif
  clientPush(detail::StreamControl::CANCEL);
  clientClose();
}

void ServerGeneratorStream::resetClientCallback(
    StreamClientCallback& clientCallback) {
  streamClientCallback_ = &clientCallback;
}

void ServerGeneratorStream::pauseStream() {
  clientPush(detail::StreamControl::PAUSE);
}

void ServerGeneratorStream::resumeStream() {
  clientPush(detail::StreamControl::RESUME);
}

void ServerGeneratorStream::processPayloads() {
  clientEventBase_->dcheckIsInEventBaseThread();
  while (!clientWait(this)) {
    for (auto messages = clientGetMessages(); !messages.empty();
         messages.pop()) {
      DCHECK(!isClientClosed());
      auto& payload = messages.front();
      if (payload.hasValue()) {
        auto alive = payload->payload || payload->isOrderedHeader
            ? streamClientCallback_->onStreamNext(std::move(payload.value()))
            : streamClientCallback_->onStreamHeaders(
                  HeadersPayload(std::move(payload->metadata)));
        if (!alive) {
          break;
        }
      } else if (payload.hasException()) {
        streamClientCallback_->onStreamError(std::move(payload.exception()));
        Ptr(this);
        return;
      } else {
        streamClientCallback_->onStreamComplete();
        Ptr(this);
        return;
      }
    }
  }
}

} // namespace detail
} // namespace thrift
} // namespace apache
