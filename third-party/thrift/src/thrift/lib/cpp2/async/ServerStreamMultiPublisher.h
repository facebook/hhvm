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

#pragma once

#include <folly/Synchronized.h>
#include <folly/Try.h>
#include <thrift/lib/cpp2/async/ServerPublisherStream.h>
#include <thrift/lib/cpp2/async/ServerStream.h>
#include <thrift/lib/cpp2/async/ServerStreamDetail.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>

namespace apache::thrift {

template <typename T, bool WithHeader = false>
class ServerStreamMultiPublisher {
  using ConditionalPayload =
      typename ServerStreamPublisher<T, WithHeader>::ConditionalPayload;
  using StreamPtr = typename detail::ServerPublisherStream<T, WithHeader>::Ptr;
  using StreamPtrMap = folly::F14FastMap<size_t, StreamPtr>;
  using EncodeToStreams = folly::Synchronized<
      folly::F14FastMap<detail::StreamElementEncoder<T>*, StreamPtrMap>>;

  static auto constexpr kPendingStreamsKey = nullptr;

 public:
  ServerStreamMultiPublisher()
      : encodeToStreams_(std::make_shared<EncodeToStreams>()) {}

  void next(ConditionalPayload payload) {
    encodeToStreams_->withRLock([=, this, payload = std::move(payload)](
                                    const auto& map) {
      auto remaining = map.size();
      for (auto& [encode, streams] : map) {
        bool canMove = --remaining == 0;
        ConditionalPayload copy = canMove ? std::move(payload) : payload;

        if (!encode) {
          publishAll(folly::Try<ConditionalPayload>(std::move(copy)), streams);
          continue;
        }

        if constexpr (WithHeader) {
          folly::Try<StreamPayload> sp =
              detail::encodeMessageVariant(encode, std::move(copy));
          if (sp->payload) {
            sp->payload->coalesce();
          }
          publishAll(std::move(sp), streams);
        } else {
          auto encoded = (*encode)(std::move(copy));
          encoded->payload->coalesce();
          publishAll(std::move(encoded), streams);
        }
      }
    });
  }

  void complete(folly::exception_wrapper ew) && {
    auto encodeToStreams =
        std::move(*std::exchange(encodeToStreams_, nullptr)->wlock());
    auto remaining = encodeToStreams.size();
    for (auto& [encode, streams] : encodeToStreams) {
      bool canMove = --remaining == 0;
      auto copy = canMove ? std::move(ew) : ew;
      if (!encode) {
        publishAll(folly::Try<ConditionalPayload>(std::move(copy)), streams);
      } else {
        publishAll((*encode)(std::move(copy)), streams);
      }
    }
  }

  void complete() && {
    auto encodeToStreams =
        std::move(*std::exchange(encodeToStreams_, nullptr)->wlock());
    for (auto& [encode, streams] : encodeToStreams) {
      if (!encode) {
        publishAll(folly::Try<ConditionalPayload>{}, streams);
      } else {
        publishAll(folly::Try<StreamPayload>{}, streams);
      }
    }
  }

  ServerStream<T> addStream() {
    return addStream([] {});
  }

  // Completion callback is optional
  // It must not call complete() on the publisher object inline
  ServerStream<T> addStream(folly::Function<void()> onStreamCompleteOrCancel) {
    auto streamIndex = createStreamIndex();
    auto cb = [streamIndex,
               userCb = std::move(onStreamCompleteOrCancel),
               encodeToStreamsWeak =
                   folly::to_weak_ptr(encodeToStreams_)]() mutable {
      removeStream(std::move(encodeToStreamsWeak), streamIndex);
      userCb();
    };
    auto stream =
        new detail::ServerPublisherStream<T, WithHeader>(std::move(cb));
    addStreamToMap(streamIndex, stream->copy());
    return ServerStream<T>(
        [streamIndex,
         encodeToStreamsWeak = folly::to_weak_ptr(encodeToStreams_),
         stream = std::unique_ptr<
             detail::ServerPublisherStream<T, WithHeader>,
             typename detail::ServerPublisherStream<T, WithHeader>::
                 CancelDeleter>(stream)](
            folly::Executor::KeepAlive<> serverExecutor,
            detail::StreamElementEncoder<T>* encode) mutable {
          auto serverStreamFactory =
              detail::ServerPublisherStream<T, WithHeader>::establishStream(
                  std::move(stream), std::move(serverExecutor), encode);
          provideEncodeFn(std::move(encodeToStreamsWeak), streamIndex, encode);
          return serverStreamFactory;
        });
  }

  ServerStreamMultiPublisher(ServerStreamMultiPublisher&&) = default;
  ServerStreamMultiPublisher& operator=(ServerStreamMultiPublisher&& other) {
    if (&other != this) {
      CHECK(!encodeToStreams_ || allStreamsCompletedOrCancelled())
          << "StreamMultiPublisher must be completed or all streams must be cancelled.";
    }
    encodeToStreams_ = std::move(other.encodeToStreams_);
    return *this;
  }

  ~ServerStreamMultiPublisher() {
    CHECK(!encodeToStreams_ || allStreamsCompletedOrCancelled())
        << "StreamMultiPublisher must be completed or all streams must be cancelled.";
  }

 private:
  template <typename Payload>
  void publishAll(folly::Try<Payload>&& payload, const StreamPtrMap& streams) {
    auto remaining = streams.size();
    for (auto& [_, stream] : streams) {
      if (--remaining == 0) {
        stream->publish(std::move(payload));
      } else if (payload.hasException()) {
        folly::exception_wrapper copy;
        payload.exception().handle(
            [&](const detail::EncodedStreamError& err) {
              copy = folly::make_exception_wrapper<detail::EncodedStreamError>(
                  err);
            },
            [&](...) { copy = payload.exception(); });
        stream->publish(folly::Try<Payload>(std::move(copy)));
      } else {
        stream->publish(folly::copy(payload));
      }
    }
  }

  void addStreamToMap(size_t streamIndex, StreamPtr stream) {
    encodeToStreams_->withWLock([&](auto& map) mutable {
      map[kPendingStreamsKey][streamIndex] = std::move(stream);
    });
  }

  static void removeStream(
      std::weak_ptr<EncodeToStreams> encodeToStreamsWeak, size_t streamIndex) {
    if (auto encodeToStreams = encodeToStreamsWeak.lock()) {
      encodeToStreams->withWLock([=](auto& map) {
        for (auto it = map.begin(); it != map.end(); it++) {
          auto& streams = it->second;
          if (streams.erase(streamIndex)) {
            if (streams.empty()) {
              map.erase(it);
            }
            break;
          }
        }
      });
    }
  }

  static void provideEncodeFn(
      std::weak_ptr<EncodeToStreams> encodeToStreamsWeak,
      size_t streamIndex,
      detail::StreamElementEncoder<T>* encodeFn) {
    if (auto encodeToStreams = encodeToStreamsWeak.lock()) {
      encodeToStreams->withWLock([=](auto& map) {
        auto& streamsMap = map[encodeFn];
        map[kPendingStreamsKey].eraseInto(
            streamIndex, [&](size_t, StreamPtr&& stream) {
              streamsMap.emplace(streamIndex, std::move(stream));
            });
        if (map[kPendingStreamsKey].empty()) {
          map.erase(kPendingStreamsKey);
        }
      });
    }
  }

  FOLLY_EXPORT size_t createStreamIndex() {
    static std::atomic<size_t> counter{0};
    return counter.fetch_add(1, std::memory_order_relaxed);
  }

  bool allStreamsCompletedOrCancelled() {
    bool ret = true;
    encodeToStreams_->withRLock([&](const auto& map) {
      for (auto& [_, streams] : map) {
        for (auto& [_, stream] : streams) {
          if (!stream->wasCancelled()) {
            ret = false;
            return;
          }
        }
      }
    });
    return ret;
  }

  std::shared_ptr<EncodeToStreams> encodeToStreams_;
};

} // namespace apache::thrift
