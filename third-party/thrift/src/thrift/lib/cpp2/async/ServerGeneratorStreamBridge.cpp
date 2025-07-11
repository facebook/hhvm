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

#include <thrift/lib/cpp2/async/ServerGeneratorStreamBridge.h>

namespace apache::thrift::detail {

// Explicitly instantiate the base of ServerGeneratorStreamBridge
template class TwoWayBridge<
    ServerGeneratorStreamBridge,
    folly::Try<StreamPayload>,
    ServerStreamConsumer,
    int64_t,
    ServerGeneratorStreamBridge>;

ServerGeneratorStreamBridge::ServerGeneratorStreamBridge(
    StreamClientCallback* clientCallback, folly::EventBase* clientEb)
    : streamClientCallback_(clientCallback), clientEventBase_(clientEb) {}

ServerGeneratorStreamBridge::~ServerGeneratorStreamBridge() {}

/* static */ ServerStreamFactory
ServerGeneratorStreamBridge::fromProducerCallback(ProducerCallback* cb) {
  return ServerStreamFactory([cb](
                                 FirstResponsePayload&& payload,
                                 StreamClientCallback* callback,
                                 folly::EventBase* clientEb,
                                 TilePtr&&,
                                 ContextStack::UniquePtr) mutable {
    DCHECK(clientEb->isInEventBaseThread());
    auto stream = new ServerGeneratorStreamBridge(callback, clientEb);
    std::ignore =
        callback->onFirstResponse(std::move(payload), clientEb, stream);
    cb->provideStream(stream->copy());
    stream->processPayloads();
  });
}

void ServerGeneratorStreamBridge::consume() {
  clientEventBase_->add([this]() { processPayloads(); });
}
void ServerGeneratorStreamBridge::canceled() {
  Ptr(this);
}

bool ServerGeneratorStreamBridge::wait(ServerStreamConsumer* consumer) {
  return serverWait(consumer);
}

void ServerGeneratorStreamBridge::publish(folly::Try<StreamPayload>&& payload) {
  serverPush(std::move(payload));
}

ServerGeneratorStreamBridge::ServerQueue
ServerGeneratorStreamBridge::getMessages() {
  return serverGetMessages();
}

bool ServerGeneratorStreamBridge::onStreamRequestN(uint64_t credits) {
  clientPush(std::move(credits));
  return true;
}

void ServerGeneratorStreamBridge::onStreamCancel() {
#if FOLLY_HAS_COROUTINES
  cancelSource_.requestCancellation();
#endif
  clientPush(detail::StreamControl::CANCEL);
  clientClose();
}

void ServerGeneratorStreamBridge::resetClientCallback(
    StreamClientCallback& clientCallback) {
  streamClientCallback_ = &clientCallback;
}

void ServerGeneratorStreamBridge::pauseStream() {
  clientPush(detail::StreamControl::PAUSE);
}

void ServerGeneratorStreamBridge::resumeStream() {
  clientPush(detail::StreamControl::RESUME);
}

void ServerGeneratorStreamBridge::processPayloads() {
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

void ServerGeneratorStreamBridge::close() {
  serverClose();
}
} // namespace apache::thrift::detail
