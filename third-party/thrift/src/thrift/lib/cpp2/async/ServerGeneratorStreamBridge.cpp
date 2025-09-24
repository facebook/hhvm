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

#include <thrift/lib/cpp/StreamEventHandler.h>
#include <thrift/lib/cpp2/async/ServerGeneratorStreamBridge.h>

namespace apache::thrift::detail {

// Explicitly instantiate the base of ServerGeneratorStreamBridge
template class TwoWayBridge<
    ServerGeneratorStreamBridge,
    folly::Try<StreamPayload>,
    QueueConsumer,
    int64_t,
    ServerGeneratorStreamBridge>;

ServerGeneratorStreamBridge::ServerGeneratorStreamBridge(
    StreamClientCallback* clientCallback,
    folly::EventBase* evb,
    std::shared_ptr<ContextStack> contextStack)
    : clientCallback_(clientCallback),
      evb_(evb),
      contextStack_(std::move(contextStack)) {}

/* static */ ServerStreamFactory
ServerGeneratorStreamBridge::fromProducerCallback(
    ProducerCallback* producerCallback) {
  return ServerStreamFactory([producerCallback](
                                 FirstResponsePayload&& payload,
                                 StreamClientCallback* clientCallback,
                                 folly::EventBase* clientEb,
                                 TilePtr&&,
                                 std::shared_ptr<ContextStack>) mutable {
    DCHECK(clientEb->isInEventBaseThread());
    auto stream = new ServerGeneratorStreamBridge(clientCallback, clientEb);
    std::ignore =
        clientCallback->onFirstResponse(std::move(payload), clientEb, stream);
    producerCallback->provideStream(stream->copy());
    stream->processClientMessages();
  });
}

void ServerGeneratorStreamBridge::consume() {
  evb_->add([this]() { processClientMessages(); });
}

void ServerGeneratorStreamBridge::canceled() {
  Ptr(this);
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
  clientCallback_ = &clientCallback;
}

void ServerGeneratorStreamBridge::pauseStream() {
  clientPush(detail::StreamControl::PAUSE);
}

void ServerGeneratorStreamBridge::resumeStream() {
  clientPush(detail::StreamControl::RESUME);
}

void ServerGeneratorStreamBridge::processClientMessages() {
  evb_->dcheckIsInEventBaseThread();
  while (!clientWait(this)) {
    for (auto messages = clientGetMessages(); !messages.empty();
         messages.pop()) {
      DCHECK(!isClientClosed());
      auto& payload = messages.front();
      if (payload.hasValue()) {
        auto alive = payload->payload || payload->isOrderedHeader
            ? clientCallback_->onStreamNext(std::move(payload.value()))
            : clientCallback_->onStreamHeaders(
                  HeadersPayload(std::move(payload->metadata)));
        if (!alive) {
          break;
        }
      } else if (payload.hasException()) {
        clientCallback_->onStreamError(std::move(payload.exception()));
        Ptr(this);
        return;
      } else {
        clientCallback_->onStreamComplete();
        Ptr(this);
        return;
      }
    }
  }
}

//
// Helper methods to encapsulate ContextStack usage
//
/*static*/ void ServerGeneratorStreamBridge::notifyStreamSubscribe(
    ContextStack* contextStack, const TileStreamGuard& interaction) {
  if (contextStack) {
    contextStack->onStreamSubscribe(
        {.interactionCreationTime = interaction.getInteractionCreationTime()});
  }
}

/*static*/ void ServerGeneratorStreamBridge::notifyStreamCancel(
    ContextStack* contextStack) {
  if (contextStack) {
    contextStack->onStreamFinally(details::STREAM_ENDING_TYPES::CANCEL);
  }
}

/*static*/ void ServerGeneratorStreamBridge::notifyStreamError(
    ContextStack* contextStack, const folly::exception_wrapper& exception) {
  if (contextStack) {
    contextStack->handleStreamErrorWrapped(exception);
    contextStack->onStreamFinally(details::STREAM_ENDING_TYPES::ERROR);
  }
}

/*static*/ void ServerGeneratorStreamBridge::notifyStreamCompletion(
    ContextStack* contextStack) {
  if (contextStack) {
    contextStack->onStreamFinally(details::STREAM_ENDING_TYPES::COMPLETE);
  }
}

/*static*/ void ServerGeneratorStreamBridge::notifyStreamPause(
    ContextStack* contextStack, details::STREAM_PAUSE_REASON reason) {
  if (contextStack) {
    contextStack->onStreamPause(reason);
  }
}

/*static*/ void ServerGeneratorStreamBridge::notifyStreamResumeReceive(
    ContextStack* contextStack) {
  if (contextStack) {
    contextStack->onStreamResumeReceive();
  }
}

/*static*/ void ServerGeneratorStreamBridge::notifyStreamCredit(
    ContextStack* contextStack, int64_t credits) {
  if (contextStack) {
    contextStack->onStreamCredit(credits);
  }
}

/*static*/ void ServerGeneratorStreamBridge::notifyStreamNext(
    ContextStack* contextStack) {
  if (contextStack) {
    contextStack->onStreamNext();
  }
}

//
// end of Helper methods to encapsulate ContextStack usage
//
} // namespace apache::thrift::detail
