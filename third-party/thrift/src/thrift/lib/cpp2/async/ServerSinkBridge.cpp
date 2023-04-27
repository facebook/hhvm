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

#include <thrift/lib/cpp2/async/ServerSinkBridge.h>

#include <folly/Overload.h>

#if FOLLY_HAS_COROUTINES
namespace apache {
namespace thrift {
namespace detail {

// Explicitly instantiate the base of ServerSinkBridge
template class TwoWayBridge<
    ServerSinkBridge,
    ClientMessage,
    CoroConsumer,
    ServerMessage,
    ServerSinkBridge>;

ServerSinkBridge::ServerSinkBridge(
    SinkConsumerImpl&& sinkConsumer,
    folly::EventBase& evb,
    SinkClientCallback* callback)
    : consumer_(std::move(sinkConsumer)),
      evb_(folly::getKeepAliveToken(&evb)),
      clientCallback_(callback) {
  interaction_ =
      TileStreamGuard::transferFrom(std::move(sinkConsumer.interaction));
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
  clientPush(folly::Try<StreamPayload>(std::move(payload)));
  return true;
}

void ServerSinkBridge::onSinkError(folly::exception_wrapper ew) {
  using apache::thrift::detail::EncodedError;
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

// start should be called on threadmanager's thread
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
  uint64_t counter = 0;
  while (true) {
    CoroConsumer consumer;
    if (serverWait(&consumer)) {
      folly::CancellationCallback cb{
          co_await folly::coro::co_current_cancellation_token,
          [&]() { serverClose(); }};
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
              clientCallback_->onFinalResponse(std::move(payload).value());
            } else {
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
    std::ignore = clientCallback_->onSinkRequestN(credits);
  }
}

void ServerSinkBridge::close() {
  clientClose();
  clientCallback_ = nullptr;
  Ptr(this);
}

} // namespace detail
} // namespace thrift
} // namespace apache
#endif
