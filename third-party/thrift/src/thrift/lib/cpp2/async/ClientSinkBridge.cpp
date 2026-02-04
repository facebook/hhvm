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

#include <thrift/lib/cpp2/async/ClientSinkBridge.h>

#include <folly/Overload.h>

namespace apache::thrift::detail {

// Explicitly instantiate the base of ClientSinkBridge
template class TwoWayBridge<
    QueueConsumer,
    ClientSinkMessageServerToClient,
    ClientSinkBridge,
    ClientSinkMessageClientToServer,
    ClientSinkBridge>;

void ClientSinkBridge::ClientDeleter::operator()(ClientSinkBridge* ptr) {
  ptr->cancel(
      folly::Try<StreamPayload>{
          folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::TApplicationExceptionType::INTERRUPTION,
              "never called sink object")});
  Deleter::operator()(ptr);
}

ClientSinkBridge::ClientSinkBridge(FirstResponseCallback* callback)
    : firstResponseCallback_(callback) {}

ClientSinkBridge::~ClientSinkBridge() {
  DCHECK(!evb_);
}

SinkClientCallback* ClientSinkBridge::create(FirstResponseCallback* callback) {
  return new ClientSinkBridge(callback);
}

void ClientSinkBridge::close() {
  serverClose();
  serverCallback_ = nullptr;
  Ptr(this);
}

bool ClientSinkBridge::wait(QueueConsumer* consumer) {
  return clientWait(consumer);
}

void ClientSinkBridge::push(folly::Try<StreamPayload>&& value) {
  clientPush(StreamMessage::PayloadOrError{std::move(value)});
}

ClientSinkBridge::ClientQueue ClientSinkBridge::getMessages() {
  return clientGetMessages();
}

#if FOLLY_HAS_COROUTINES

folly::coro::Task<folly::Try<StreamPayload>> ClientSinkBridge::sink(
    folly::coro::AsyncGenerator<folly::Try<StreamPayload>&&> generator) {
  const auto& clientCancelToken =
      co_await folly::coro::co_current_cancellation_token;
  auto mergedToken = folly::cancellation_token_merge(
      serverCancelSource_.getToken(), clientCancelToken);
  auto makeCancellationCallback = [&] {
    return folly::CancellationCallback{
        clientCancelToken, [&]() {
          if (auto* cancelledCb = cancelClientWait()) {
            cancelledCb->canceled();
          }
        }};
  };
  SCOPE_EXIT {
    evb_.reset();
  };

  for (uint64_t credits = 0; !serverCancelSource_.isCancellationRequested();
       credits--) {
    if (credits == 0) {
      if (CoroConsumer consumer; clientWait(&consumer)) {
        auto cb = makeCancellationCallback();
        co_await consumer.wait();
      }

      for (auto queue = clientGetMessages(); !queue.empty(); queue.pop()) {
        auto& message = queue.front();
        bool gotResponse = folly::variant_match(
            message,
            [&](StreamMessage::RequestN& requestN) {
              credits += requestN.n;
              return false;
            },
            [](StreamMessage::PayloadOrError&) { return true; });
        if (gotResponse) {
          co_return std::move(
              std::get<StreamMessage::PayloadOrError>(message)
                  .streamPayloadTry);
        }
      }

      if (clientCancelToken.isCancellationRequested()) {
        clientPush(
            StreamMessage::PayloadOrError{
                folly::Try<apache::thrift::StreamPayload>(
                    rocket::RocketException(rocket::ErrorCode::CANCELED))});
        co_yield folly::coro::co_stopped_may_throw;
      }
    }

    auto item = co_await folly::coro::co_withCancellation(
        mergedToken, generator.next());
    if (serverCancelSource_.isCancellationRequested()) {
      break;
    }

    if (clientCancelToken.isCancellationRequested()) {
      clientPush(
          StreamMessage::PayloadOrError{
              folly::Try<apache::thrift::StreamPayload>(
                  rocket::RocketException(rocket::ErrorCode::CANCELED))});
      co_yield folly::coro::co_stopped_may_throw;
    }

    if (item.has_value()) {
      if ((*item).hasValue()) {
        clientPush(StreamMessage::PayloadOrError{std::move(*item)});
      } else {
        clientPush(StreamMessage::PayloadOrError{std::move(*item)});
        // AsyncGenerator who serialized and yield the exception also in
        // charge of propagating it back to user, just return empty Try
        // here.
        co_return folly::Try<StreamPayload>();
      }
    } else {
      clientPush(StreamMessage::Complete{});
      // release generator
      generator = {};
      break;
    }
  }

  // Generator is done or server cancel source signalled: wait for final respose
  while (true) {
    if (CoroConsumer consumer; clientWait(&consumer)) {
      auto cb = makeCancellationCallback();
      co_await consumer.wait();
    }

    for (auto queue = clientGetMessages(); !queue.empty(); queue.pop()) {
      auto& message = queue.front();
      if (auto* payloadOrError =
              std::get_if<StreamMessage::PayloadOrError>(&message)) {
        co_return std::move(payloadOrError->streamPayloadTry);
      }
    }

    // If we got here because the generator is done and the server hasn't
    // responded yet we'd like to send it a canceled error like we do above, but
    // the sink contract does not allow it.
    co_await folly::coro::co_safe_point;
  }
}

#endif

void ClientSinkBridge::cancel(folly::Try<StreamPayload> payload) {
  CHECK(payload.hasException());
  DCHECK(evb_);
  clientPush(StreamMessage::PayloadOrError{std::move(payload)});
  evb_.reset();
}

// SinkClientCallback method
bool ClientSinkBridge::onFirstResponse(
    FirstResponsePayload&& firstPayload,
    folly::EventBase* evb,
    SinkServerCallback* serverCallback) {
  auto firstResponseCallback = firstResponseCallback_;
  serverCallback_ = serverCallback;
  evb_ = folly::getKeepAliveToken(evb);
  bool scheduledWait = serverWait(this);
  DCHECK(scheduledWait);
  firstResponseCallback->onFirstResponse(
      std::move(firstPayload), ClientPtr(copy().release()));
  return true;
}

void ClientSinkBridge::onFirstResponseError(folly::exception_wrapper ew) {
  firstResponseCallback_->onFirstResponseError(std::move(ew));
  Ptr(this);
}

void ClientSinkBridge::onFinalResponse(StreamPayload&& payload) {
  serverPush(
      StreamMessage::PayloadOrError{
          folly::Try<StreamPayload>(std::move(payload))});
  serverCancelSource_.requestCancellation();
  close();
}

void ClientSinkBridge::onFinalResponseError(folly::exception_wrapper ew) {
  using apache::thrift::detail::EncodedError;
  auto rex = ew.get_mutable_exception<rocket::RocketException>();
  auto payload = rex && rex->hasErrorData()
      ? folly::Try<StreamPayload>(EncodedError(rex->moveErrorData()))
      : folly::Try<StreamPayload>(std::move(ew));
  serverPush(StreamMessage::PayloadOrError{std::move(payload)});
  serverCancelSource_.requestCancellation();
  close();
}

bool ClientSinkBridge::onSinkRequestN(int32_t n) {
  serverPush(StreamMessage::RequestN{n});
  return true;
}

void ClientSinkBridge::resetServerCallback(SinkServerCallback& serverCallback) {
  serverCallback_ = &serverCallback;
}

void ClientSinkBridge::consume() {
  DCHECK(evb_);
  evb_->runInEventBaseThread(
      [self = copy()]() mutable { self->processServerMessages(); });
}

void ClientSinkBridge::processServerMessages() {
  if (!serverCallback_) {
    return;
  }

  do {
    for (auto messages = serverGetMessages(); !messages.empty();
         messages.pop()) {
      auto& message = messages.front();
      bool terminated = folly::variant_match(
          message,
          [&](StreamMessage::PayloadOrError& payloadOrError) {
            auto& payload = payloadOrError.streamPayloadTry;
            if (payload.hasException()) {
              serverCallback_->onSinkError(std::move(payload).exception());
              close();
              return true;
            } else {
              return !serverCallback_->onSinkNext(std::move(payload).value());
            }
          },
          [&](StreamMessage::Complete) {
            return !serverCallback_->onSinkComplete();
          });

      if (terminated) {
        return;
      }
    }
  } while (!serverWait(this));
}

bool ClientSinkBridge::hasServerCancelled() {
  return serverCancelSource_.isCancellationRequested();
}

} // namespace apache::thrift::detail
