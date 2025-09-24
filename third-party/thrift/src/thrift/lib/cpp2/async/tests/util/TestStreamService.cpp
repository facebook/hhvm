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

#include <thrift/lib/cpp2/async/tests/util/TestStreamService.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>

#include <gtest/gtest.h>
#include <folly/coro/AsyncScope.h>

namespace apache::thrift::detail::test {
class ReadyCallback final : public QueueConsumer {
 public:
  void consume() override { baton.post(); }
  void canceled() override { LOG(WARNING) << "Server stream is cancelled"; }

  void wait() { baton.wait(); }

  folly::Baton<> baton;
};

ServerStream<int32_t> TestStreamGeneratorService::range(
    int32_t from, int32_t to) {
  for (int i = from; i <= to; i++) {
    co_yield int(i);
  }
}

ServerStream<int32_t> TestStreamGeneratorService::rangeThrow(
    int32_t from, int32_t to) {
  for (int i = from; i <= to; i++) {
    co_yield int(i);
  }
  throw std::runtime_error("I am a search bar");
}

ServerStream<int32_t> TestStreamGeneratorService::rangeThrowUDE(
    int32_t from, int32_t to) {
  for (int i = from; i <= to; i++) {
    co_yield int(i);
  }
  throw UserDefinedException();
}

ServerStream<int32_t> TestStreamPublisherService::range(
    int32_t from, int32_t to) {
  auto [stream, publisher] = ServerStream<int32_t>::createPublisher([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(i);
  }
  std::move(publisher).complete();

  return std::move(stream);
}

ServerStream<int32_t> TestStreamPublisherService::rangeThrow(
    int32_t from, int32_t to) {
  auto [stream, publisher] = ServerStream<int32_t>::createPublisher([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(i);
  }
  std::move(publisher).complete(std::runtime_error("I am a search bar"));

  return std::move(stream);
}

ServerStream<int32_t> TestStreamPublisherService::rangeThrowUDE(
    int32_t from, int32_t to) {
  auto [stream, publisher] = ServerStream<int32_t>::createPublisher([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(i);
  }
  std::move(publisher).complete(UserDefinedException());

  return std::move(stream);
}

using RichPayload = RichPayloadToSend<int32_t>;
using MessageVariant = MessageVariant<int32_t>;

ServerStream<int32_t> TestStreamGeneratorWithHeaderService::range(
    int32_t from, int32_t to) {
  return folly::coro::co_invoke(
      [=]() -> folly::coro::AsyncGenerator<MessageVariant&&> {
        for (int i = from; i <= to; i++) {
          co_yield RichPayload{i, {{"val", std::to_string(i)}}, {}};
          co_yield UnorderedHeader{{{"val", std::to_string(i)}}};
          co_yield OrderedHeader{{{"val", std::to_string(i)}}};
        }
      });
}

ServerStream<int32_t> TestStreamGeneratorWithHeaderService::rangeThrow(
    int32_t from, int32_t to) {
  return folly::coro::co_invoke(
      [=]() -> folly::coro::AsyncGenerator<MessageVariant&&> {
        for (int i = from; i <= to; i++) {
          co_yield RichPayload{i, {{"val", std::to_string(i)}}, {}};
          co_yield UnorderedHeader{{{"val", std::to_string(i)}}};
          co_yield OrderedHeader{{{"val", std::to_string(i)}}};
        }
        throw std::runtime_error("I am a search bar");
      });
}

ServerStream<int32_t> TestStreamGeneratorWithHeaderService::rangeThrowUDE(
    int32_t from, int32_t to) {
  return folly::coro::co_invoke(
      [=]() -> folly::coro::AsyncGenerator<MessageVariant&&> {
        for (int i = from; i <= to; i++) {
          co_yield RichPayload{i, {{"val", std::to_string(i)}}, {}};
          co_yield UnorderedHeader{{{"val", std::to_string(i)}}};
          co_yield OrderedHeader{{{"val", std::to_string(i)}}};
        }
        throw UserDefinedException();
      });
}

ServerStream<int32_t> TestStreamPublisherWithHeaderService::range(
    int32_t from, int32_t to) {
  auto [stream, publisher] =
      ServerStream<int32_t>::createPublisherWithHeader([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(RichPayload{i, {{"val", std::to_string(i)}}, {}});
    publisher.next(UnorderedHeader{{{"val", std::to_string(i)}}});
    publisher.next(OrderedHeader{{{"val", std::to_string(i)}}});
  }
  std::move(publisher).complete();

  return std::move(stream);
}

ServerStream<int32_t> TestStreamPublisherWithHeaderService::rangeThrow(
    int32_t from, int32_t to) {
  auto [stream, publisher] =
      ServerStream<int32_t>::createPublisherWithHeader([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(RichPayload{i, {{"val", std::to_string(i)}}, {}});
    publisher.next(UnorderedHeader{{{"val", std::to_string(i)}}});
    publisher.next(OrderedHeader{{{"val", std::to_string(i)}}});
  }
  std::move(publisher).complete(std::runtime_error("I am a search bar"));

  return std::move(stream);
}

ServerStream<int32_t> TestStreamPublisherWithHeaderService::rangeThrowUDE(
    int32_t from, int32_t to) {
  auto [stream, publisher] =
      ServerStream<int32_t>::createPublisherWithHeader([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(RichPayload{i, {{"val", std::to_string(i)}}, {}});
    publisher.next(UnorderedHeader{{{"val", std::to_string(i)}}});
    publisher.next(OrderedHeader{{{"val", std::to_string(i)}}});
  }
  std::move(publisher).complete(UserDefinedException());

  return std::move(stream);
}

ServerStream<int32_t> TestStreamMultiPublisherService::range(
    int32_t from, int32_t to) {
  return range(from, to, false, folly::exception_wrapper());
}

ServerStream<int32_t> TestStreamMultiPublisherService::rangeWaitForCancellation(
    int32_t from, int32_t to) {
  return range(from, to, true, folly::exception_wrapper());
}

ServerStream<int32_t> TestStreamMultiPublisherService::rangeThrow(
    int32_t from, int32_t to) {
  return range(from, to, false, std::runtime_error("oops"));
}

ServerStream<int32_t> TestStreamMultiPublisherService::rangeThrowUDE(
    int32_t from, int32_t to) {
  return range(from, to, false, UserDefinedException());
}

ServerStream<int32_t>
TestStreamMultiPublisherService::uncompletedPublisherDestructor() {
  auto stream = multipub_.addStream();
  EXPECT_DEATH(
      [_ = std::move(multipub_)] {}(),
      "StreamMultiPublisher must be completed or all streams must be cancelled");
  return stream;
}

ServerStream<int32_t>
TestStreamMultiPublisherService::uncompletedPublisherMoveAssignment() {
  auto stream = multipub_.addStream();
  EXPECT_DEATH(
      multipub_ = ServerStreamMultiPublisher<int32_t>(),
      "StreamMultiPublisher must be completed or all streams must be cancelled");
  return stream;
}

ServerStream<int32_t> TestStreamMultiPublisherService::range(
    int32_t from,
    int32_t to,
    bool waitForCancellation,
    folly::exception_wrapper ew) {
  auto stream = multipub_.addStream([&] { --activeStreams_; });
  if (++activeStreams_ == 5) {
    getAsyncScope()->add(co_withExecutor(
        getEventBase(),
        folly::coro::co_invoke(
            [=, this, ew = std::move(ew)]() mutable -> folly::coro::Task<void> {
              for (int i = from; i <= to; i++) {
                if (waitForCancellation && i == from + 1) {
                  co_await *waitForCancellation_;
                }
                multipub_.next(i);
                co_await folly::coro::co_reschedule_on_current_executor;
              }
              if (ew) {
                std::move(multipub_).complete(std::move(ew));
              } else {
                std::move(multipub_).complete();
              }
              EXPECT_EQ(0, activeStreams_);
            })));
  }
  return stream;
}

ServerStream<int32_t> TestStreamMultiPublisherWithHeaderService::range(
    int32_t from, int32_t to) {
  return range(from, to, false, folly::exception_wrapper());
}

ServerStream<int32_t>
TestStreamMultiPublisherWithHeaderService::rangeWaitForCancellation(
    int32_t from, int32_t to) {
  return range(from, to, true, folly::exception_wrapper());
}

ServerStream<int32_t> TestStreamMultiPublisherWithHeaderService::rangeThrow(
    int32_t from, int32_t to) {
  return range(from, to, false, std::runtime_error("oops"));
}

ServerStream<int32_t> TestStreamMultiPublisherWithHeaderService::rangeThrowUDE(
    int32_t from, int32_t to) {
  return range(from, to, false, UserDefinedException());
}

ServerStream<int32_t>
TestStreamMultiPublisherWithHeaderService::rangePassiveSubscription() {
  return multipub_.addStream([] {});
}

ServerStream<int32_t>
TestStreamMultiPublisherWithHeaderService::uncompletedPublisherDestructor() {
  auto stream = multipub_.addStream();
  EXPECT_DEATH(
      [_ = std::move(multipub_)] {}(),
      "StreamMultiPublisher must be completed or all streams must be cancelled");
  return stream;
}
ServerStream<int32_t> TestStreamMultiPublisherWithHeaderService::
    uncompletedPublisherMoveAssignment() {
  auto stream = multipub_.addStream();
  auto multipub = ServerStreamMultiPublisher<int32_t, true>();
  EXPECT_DEATH(
      multipub_ = std::move(multipub),
      "StreamMultiPublisher must be completed or all streams must be cancelled");
  return stream;
}

ServerStream<int32_t> TestStreamMultiPublisherWithHeaderService::range(
    int32_t from,
    int32_t to,
    bool waitForCancellation,
    folly::exception_wrapper ew) {
  auto stream = multipub_.addStream([&] { --activeStreams_; });
  if (++activeStreams_ == 5) {
    getAsyncScope()->add(co_withExecutor(
        getEventBase(),
        folly::coro::co_invoke(
            [=, this, ew = std::move(ew)]() mutable -> folly::coro::Task<void> {
              for (int i = from; i <= to; i++) {
                if (waitForCancellation && i == from + 1) {
                  co_await *waitForCancellation_;
                }
                multipub_.next(
                    RichPayload{i, {{"val", std::to_string(i)}}, {}});
                multipub_.next(UnorderedHeader{{{"val", std::to_string(i)}}});
                multipub_.next(OrderedHeader{{{"val", std::to_string(i)}}});
                co_await folly::coro::co_reschedule_on_current_executor;
              }
              if (ew) {
                std::move(multipub_).complete(std::move(ew));
              } else {
                std::move(multipub_).complete();
              }
              EXPECT_EQ(0, activeStreams_);
            })));
  }
  return stream;
}

class TestProducerCallback
    : public ServerGeneratorStreamBridge::ProducerCallback {
 public:
  TestProducerCallback(
      int32_t from,
      int32_t to,
      folly::exception_wrapper&& ew,
      folly::Executor::KeepAlive<> executor,
      StreamElementEncoder<int32_t>* encoder)
      : from_(from),
        to_(to),
        ew_(std::move(ew)),
        executor_(executor),
        encoder_(encoder) {}

  void provideStream(ServerGeneratorStreamBridge::Ptr stream) override {
    stream_ = std::move(stream);
    executor_->add([this] { run(); });
  }

  void run() {
    SCOPE_EXIT {
      stream_->serverClose();
      delete this;
    };
    for (int i = from_; i <= to_; ++i) {
      if (credits_ == 0 && updateCreditsOrCancel()) {
        return;
      }
      stream_->serverPush((*encoder_)(int(i)));
      --credits_;
    }
    if (ew_) {
      stream_->serverPush((*encoder_)(std::move(ew_)));
    } else {
      stream_->serverPush({});
    }
  }

  // returns true iff stream was cancelled by client
  bool updateCreditsOrCancel() {
    ReadyCallback consumer;
    if (stream_->serverWait(&consumer)) {
      consumer.baton.wait();
    }

    auto queue = stream_->serverGetMessages();
    while (!queue.empty()) {
      auto next = queue.front();
      queue.pop();
      switch (next) {
        case StreamControl::CANCEL:
          return true;
        case StreamControl::PAUSE:
        case StreamControl::RESUME:
          // ignore pause/resume events
          continue;
        default:
          credits_ += next;
          break;
      }
    }
    return false;
  }

 private:
  int32_t from_;
  int32_t to_;
  uint64_t credits_{0};
  folly::exception_wrapper ew_;
  ServerGeneratorStreamBridge::Ptr stream_;
  folly::Executor::KeepAlive<> executor_;
  StreamElementEncoder<int32_t>* encoder_;
};

ServerStream<int32_t> TestStreamProducerCallbackService::range(
    int32_t from, int32_t to) {
  return ServerStream<int32_t>(
      [from, to](
          folly::Executor::KeepAlive<> executor,
          StreamElementEncoder<int32_t>* encoder) mutable {
        return ServerGeneratorStreamBridge::fromProducerCallback(
            new TestProducerCallback(from, to, {}, executor, encoder));
      });
}

ServerStream<int32_t> TestStreamProducerCallbackService::rangeThrow(
    int32_t from, int32_t to) {
  return ServerStream<int32_t>(
      [from, to](
          folly::Executor::KeepAlive<> executor,
          StreamElementEncoder<int32_t>* encoder) mutable {
        return ServerGeneratorStreamBridge::fromProducerCallback(
            new TestProducerCallback(
                from,
                to,
                folly::make_exception_wrapper<std::runtime_error>(
                    "I am a search bar"),
                executor,
                encoder));
      });
}

ServerStream<int32_t> TestStreamProducerCallbackService::rangeThrowUDE(
    int32_t from, int32_t to) {
  return ServerStream<int32_t>(
      [from, to](
          folly::Executor::KeepAlive<> executor,
          StreamElementEncoder<int32_t>* encoder) mutable {
        return ServerGeneratorStreamBridge::fromProducerCallback(
            new TestProducerCallback(
                from,
                to,
                folly::make_exception_wrapper<UserDefinedException>(),
                executor,
                encoder));
      });
}

ServerStream<int32_t> TestStreamClientCallbackService::range(
    int32_t from, int32_t to) {
  auto [stream, publisher] = ServerStream<int32_t>::createPublisher([] {});
  for (int i = from; i <= to; i++) {
    publisher.next(i);
  }
  streamPublisher_ = std::move(publisher);
  return std::move(stream);
}

folly::coro::Task<int32_t> TestStreamClientCallbackService::co_test() {
  rocket::RocketStreamClientCallback* cb =
      static_cast<rocket::RocketStreamClientCallback*>(
          streamPublisher_->impl_->streamClientCallback_);
  EXPECT_EQ("range", cb->getRpcMethodName());
  std::move(*streamPublisher_).complete();
  co_return 0;
}

} // namespace apache::thrift::detail::test
