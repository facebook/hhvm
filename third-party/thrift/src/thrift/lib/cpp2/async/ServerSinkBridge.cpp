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
#include <thrift/lib/cpp2/async/ServerSinkBridge.h>
#include <thrift/lib/cpp2/logging/ThriftEvent.h>
#include <thrift/lib/cpp2/logging/ThriftSinkLog.h>

#include <folly/Overload.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift::detail {

// Explicitly instantiate the base of ServerSinkBridge
template class TwoWayBridge<
    ServerSinkBridge,
    ServerSinkMessageServerToClient,
    QueueConsumer,
    ServerSinkMessageClientToServer,
    ServerSinkBridge>;

ServerSinkBridge::ServerSinkBridge(
    SinkBridgeContext&& sinkConsumer,
    folly::EventBase& evb,
    SinkClientCallback* callback)
    : consumer_(std::move(sinkConsumer)),
      sinkLog_(std::move(consumer_.sinkLog)),
      evb_(folly::getKeepAliveToken(&evb)),
      clientCallback_(callback) {
  interaction_ =
      TileStreamGuard::transferFrom(std::move(consumer_.interaction));
  bool scheduledWait = clientWait(this);
  DCHECK(scheduledWait);
}

ServerSinkBridge::~ServerSinkBridge() {}

ServerSinkBridge::Ptr ServerSinkBridge::create(
    SinkBridgeContext&& sinkConsumer,
    folly::EventBase& evb,
    SinkClientCallback* callback) {
  return (new ServerSinkBridge(std::move(sinkConsumer), evb, callback))->copy();
}

// SinkServerCallback method
bool ServerSinkBridge::onSinkNext(StreamPayload&& payload) {
  uint64_t payloadBytes =
      payload.payload ? payload.payload->computeChainDataLength() : 0;
  notifySinkNext(payloadBytes);
  clientPush(
      StreamMessage::PayloadOrError{
          folly::Try<StreamPayload>(std::move(payload))});
  return true;
}

void ServerSinkBridge::onSinkError(folly::exception_wrapper ew) {
  using apache::thrift::detail::EncodedError;
  notifySinkError(ew);
  if (sinkLog_) {
    sinkLog_->log(detail::SinkCompleteEvent{SinkEndReason::ERROR});
  }
  auto rex = ew.get_mutable_exception<rocket::RocketException>();
  auto payload = rex
      ? folly::Try<StreamPayload>(EncodedError(rex->moveErrorData()))
      : folly::Try<StreamPayload>(std::move(ew));
  clientPush(StreamMessage::PayloadOrError{std::move(payload)});
  close();
}

bool ServerSinkBridge::onSinkComplete() {
  clientPush(StreamMessage::Complete{});
  sinkComplete_ = true;
  return true;
}

void ServerSinkBridge::resetClientCallback(SinkClientCallback& clientCallback) {
  DCHECK(clientCallback_);
  clientCallback_ = &clientCallback;
}

// TwoWayBridge consumer
void ServerSinkBridge::consume() {
  evb_->runInEventBaseThread(
      [self = copy()]() { self->processClientMessages(); });
}

void ServerSinkBridge::processClientMessages() {
  if (!clientCallback_) {
    return;
  }

  uint64_t credits = 0;
  do {
    for (auto messages = clientGetMessages(); !messages.empty();
         messages.pop()) {
      auto& message = messages.front();
      bool terminated = folly::variant_match(
          message,
          [&](StreamMessage::PayloadOrError& payloadOrError) {
            auto& payload = payloadOrError.streamPayloadTry;
            if (payload.hasValue()) {
              notifySinkFinally(details::SINK_ENDING_TYPES::COMPLETE);
              if (sinkLog_) {
                sinkLog_->log(
                    detail::SinkCompleteEvent{SinkEndReason::COMPLETE});
              }
              clientCallback_->onFinalResponse(std::move(payload).value());
            } else {
              notifySinkError(
                  payload.exception(),
                  details::SINK_ENDING_TYPES::COMPLETE_WITH_ERROR);
              if (sinkLog_) {
                sinkLog_->log(
                    detail::SinkCompleteEvent{
                        SinkEndReason::COMPLETE_WITH_ERROR});
              }
              clientCallback_->onFinalResponseError(
                  std::move(payload).exception());
            }
            return true;
          },
          [&](StreamMessage::RequestN requestN) {
            credits += requestN.n;
            return false;
          });
      if (terminated) {
        close();
        return;
      }
    }
  } while (!clientWait(this));

  if (!sinkComplete_ && credits > 0) {
    notifySinkCredit(credits);
    std::ignore = clientCallback_->onSinkRequestN(credits);
  }
}

void ServerSinkBridge::close() {
  clientClose();
  clientCallback_ = nullptr;
  Ptr(this);
}

// Helper methods to encapsulate ContextStack usage
void ServerSinkBridge::notifySinkSubscribe() {
  if (const auto& contextStack = consumer_.contextStack) {
    StreamEventHandler::StreamContext streamCtx;
    if (interaction_.hasTile()) {
      streamCtx.interactionCreationTime =
          interaction_.getInteractionCreationTime();
    }
    contextStack->onSinkSubscribe(std::move(streamCtx));
  }
  if (sinkLog_) {
    sinkLog_->log(detail::SinkSubscribeEvent{});
  }
}

void ServerSinkBridge::notifySinkNext(uint64_t payloadBytes) {
  if (const auto& contextStack = consumer_.contextStack) {
    contextStack->onSinkNext();
  }
  if (sinkLog_) {
    sinkLog_->log(detail::SinkNextEvent{payloadBytes});
  }
}

void ServerSinkBridge::notifySinkFinally(
    details::SINK_ENDING_TYPES endingType) {
  if (const auto& contextStack = consumer_.contextStack) {
    contextStack->onSinkFinally(endingType);
  }
}

void ServerSinkBridge::notifySinkError(
    const folly::exception_wrapper& exception,
    details::SINK_ENDING_TYPES endingType) {
  if (const auto& contextStack = consumer_.contextStack) {
    contextStack->handleSinkError(exception);
    contextStack->onSinkFinally(endingType);
  }
}

void ServerSinkBridge::notifySinkCredit(uint64_t credits) {
  if (const auto& contextStack = consumer_.contextStack) {
    contextStack->onSinkCredit(credits);
  }
  if (sinkLog_) {
    sinkLog_->log(detail::SinkCreditEvent{static_cast<uint32_t>(credits)});
  }
}

void ServerSinkBridge::notifySinkConsumed() {
  if (const auto& contextStack = consumer_.contextStack) {
    contextStack->onSinkConsumed();
  }
  if (sinkLog_) {
    sinkLog_->log(detail::SinkConsumedEvent{});
  }
}

void ServerSinkBridge::notifySinkCancel() {
  if (const auto& contextStack = consumer_.contextStack) {
    contextStack->onSinkCancel();
  }
  if (sinkLog_) {
    sinkLog_->log(detail::SinkCancelEvent{});
  }
}

} // namespace apache::thrift::detail

#endif
