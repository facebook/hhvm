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

#include <folly/Overload.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift::detail {

// Explicitly instantiate the base of ServerSinkBridge
template class TwoWayBridge<
    ServerSinkBridge,
    std::variant<folly::Try<StreamPayload>, uint64_t>,
    CoroConsumer,
    folly::Try<StreamPayload>,
    ServerSinkBridge>;

ServerSinkBridge::ServerSinkBridge(
    SinkConsumerImpl&& sinkConsumer,
    folly::EventBase& evb,
    SinkClientCallback* callback)
    : consumer_(std::move(sinkConsumer)),
      evb_(folly::getKeepAliveToken(&evb)),
      clientCallback_(callback) {
  interaction_ =
      TileStreamGuard::transferFrom(std::move(consumer_.interaction));
  bool scheduledWait = clientWait(this);
  DCHECK(scheduledWait);
}

ServerSinkBridge::~ServerSinkBridge() {}

ServerSinkBridge::Ptr ServerSinkBridge::create(
    SinkConsumerImpl&& sinkConsumer,
    folly::EventBase& evb,
    SinkClientCallback* callback) {
  return (new ServerSinkBridge(std::move(sinkConsumer), evb, callback))->copy();
}

// SinkServerCallback method
bool ServerSinkBridge::onSinkNext(StreamPayload&& payload) {
  notifySinkNext();
  clientPush(folly::Try<StreamPayload>(std::move(payload)));
  return true;
}

void ServerSinkBridge::onSinkError(folly::exception_wrapper ew) {
  using apache::thrift::detail::EncodedError;
  notifySinkError(ew);
  auto rex = ew.get_exception<rocket::RocketException>();
  auto payload = rex
      ? folly::Try<StreamPayload>(EncodedError(rex->moveErrorData()))
      : folly::Try<StreamPayload>(std::move(ew));
  clientPush(std::move(payload));
  close();
}

bool ServerSinkBridge::onSinkComplete() {
  clientPush({});
  sinkComplete_ = true;
  return true;
}

void ServerSinkBridge::resetClientCallback(SinkClientCallback& clientCallback) {
  DCHECK(clientCallback_);
  clientCallback_ = &clientCallback;
}

// start should be called on thread manager's thread
folly::coro::Task<void> ServerSinkBridge::start() {
  serverPush(consumer_.bufferSize);
  folly::Try<StreamPayload> finalResponse =
      co_await consumer_.consumer(makeGenerator());

  if (clientException_) {
    co_return;
  }

  serverPush(std::move(finalResponse));
}

// TwoWayBridge consumer
void ServerSinkBridge::consume() {
  evb_->runInEventBaseThread(
      [self = copy()]() { self->processClientMessages(); });
}

folly::coro::AsyncGenerator<folly::Try<StreamPayload>&&>
ServerSinkBridge::makeGenerator() {
  notifySinkSubscribe();

  uint64_t counter = 0;
  while (true) {
    CoroConsumer consumer;
    if (serverWait(&consumer)) {
      folly::CancellationCallback cb{
          co_await folly::coro::co_current_cancellation_token, [&]() {
            notifySinkCancel();
            serverClose();
          }};
      co_await consumer.wait();
    }
    co_await folly::coro::co_safe_point;
    for (auto messages = serverGetMessages(); !messages.empty();
         messages.pop()) {
      folly::Try<StreamPayload> payload = std::move(messages.front());
      if (!payload.hasValue() && !payload.hasException()) {
        co_return;
      }

      if (payload.hasException()) {
        clientException_ = true;
        co_yield std::move(payload);
        co_return;
      }

      co_yield std::move(payload);
      notifySinkConsumed();
      counter++;
      if (counter > consumer_.bufferSize / 2) {
        serverPush(counter);
        counter = 0;
      }
    }
  }
}

void ServerSinkBridge::processClientMessages() {
  if (!clientCallback_) {
    return;
  }

  uint64_t credits = 0;
  do {
    for (auto messages = clientGetMessages(); !messages.empty();
         messages.pop()) {
      bool terminated = false;
      auto& message = messages.front();
      folly::variant_match(
          message,
          [&](folly::Try<StreamPayload>& payload) {
            terminated = true;
            if (payload.hasValue()) {
              notifySinkFinally(details::SINK_ENDING_TYPES::COMPLETE);
              clientCallback_->onFinalResponse(std::move(payload).value());
            } else {
              notifySinkError(
                  payload.exception(),
                  details::SINK_ENDING_TYPES::COMPLETE_WITH_ERROR);
              clientCallback_->onFinalResponseError(
                  std::move(payload).exception());
            }
          },
          [&](int64_t n) { credits += n; });
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
}

void ServerSinkBridge::notifySinkNext() {
  if (const auto& contextStack = consumer_.contextStack) {
    contextStack->onSinkNext();
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
}

void ServerSinkBridge::notifySinkConsumed() {
  if (const auto& contextStack = consumer_.contextStack) {
    contextStack->onSinkConsumed();
  }
}

void ServerSinkBridge::notifySinkCancel() {
  if (const auto& contextStack = consumer_.contextStack) {
    contextStack->onSinkCancel();
  }
}

} // namespace apache::thrift::detail

#endif
