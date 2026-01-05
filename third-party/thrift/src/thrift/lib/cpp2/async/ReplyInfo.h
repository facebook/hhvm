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

#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/ServerBiDiStreamFactory.h>
#include <thrift/lib/cpp2/async/ServerSinkFactory.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache::thrift {

class QueueReplyInfo {
 public:
  QueueReplyInfo(
      ResponseChannelRequest::UniquePtr req,
      ResponsePayload&& response,
      folly::Optional<uint32_t> crc32c)
      : req_(std::move(req)), response_(std::move(response)), crc32c_(crc32c) {}

  void operator()() noexcept {
    req_->sendReply(std::move(response_), nullptr, crc32c_);
  }

  ResponseChannelRequest::UniquePtr req_;
  ResponsePayload response_;
  folly::Optional<uint32_t> crc32c_;
};

class StreamReplyInfo {
 public:
  StreamReplyInfo(
      ResponseChannelRequest::UniquePtr req,
      apache::thrift::detail::ServerStreamFactory stream,
      ResponsePayload&& response,
      folly::Optional<uint32_t> crc32c)
      : req_(std::move(req)),
        stream_(std::move(stream)),
        response_(std::move(response)),
        crc32c_(crc32c) {}

  void operator()() noexcept {
    req_->sendStreamReply(std::move(response_), std::move(stream_), crc32c_);
  }

  ResponseChannelRequest::UniquePtr req_;
  apache::thrift::detail::ServerStreamFactory stream_;
  ResponsePayload response_;
  folly::Optional<uint32_t> crc32c_;
};

class SinkConsumerReplyInfo {
 public:
  SinkConsumerReplyInfo(
      ResponseChannelRequest::UniquePtr req,
      apache::thrift::detail::ServerSinkFactory sinkFactory,
      ResponsePayload&& response,
      folly::Optional<uint32_t> crc32c)
      : req_(std::move(req)),
        sinkFactory_(std::move(sinkFactory)),
        response_(std::move(response)),
        crc32c_(crc32c) {}

  void operator()() noexcept {
#if FOLLY_HAS_COROUTINES
    req_->sendSinkReply(std::move(response_), std::move(sinkFactory_), crc32c_);
#endif
  }

  ResponseChannelRequest::UniquePtr req_;
  apache::thrift::detail::ServerSinkFactory sinkFactory_;
  ResponsePayload response_;
  folly::Optional<uint32_t> crc32c_;
};

class TilePromiseReplyInfo {
 public:
  TilePromiseReplyInfo(
      Cpp2ConnContext* connCtx,
      int64_t interactionId,
      TilePtr interaction,
      std::unique_ptr<Tile> ptr,
      concurrency::ThreadManager* tm,
      folly::EventBase* eb,
      folly::Executor::KeepAlive<> executor,
      bool isRPEnabled)
      : connCtx_(connCtx),
        interactionId_(interactionId),
        interaction_(std::move(interaction)),
        ptr_(std::move(ptr)),
        tm_(tm),
        eb_(eb),
        executor_(std::move(executor)),
        isRPEnabled_(isRPEnabled) {}

  void operator()() noexcept {
    TilePtr tile{ptr_.release(), eb_};
    DCHECK(dynamic_cast<TilePromise*>(interaction_.get()));
    if (isRPEnabled_) {
      static_cast<TilePromise&>(*interaction_).fulfill(*tile, executor_, *eb_);
    } else {
      static_cast<TilePromise&>(*interaction_).fulfill(*tile, tm_, *eb_);
    }
    connCtx_->tryReplaceTile(interactionId_, std::move(tile));
  }

  Cpp2ConnContext* connCtx_;
  int64_t interactionId_;
  TilePtr interaction_;
  std::unique_ptr<Tile> ptr_;
  concurrency::ThreadManager* tm_;
  folly::EventBase* eb_;
  folly::Executor::KeepAlive<> executor_;
  bool isRPEnabled_;
};

class BiDiStreamReplyInfo {
 public:
  explicit BiDiStreamReplyInfo(
      ResponseChannelRequest::UniquePtr req,
      detail::ServerBiDiStreamFactory serverBiDiStreamFactory,
      ResponsePayload&& response,
      folly::Optional<uint32_t> crc32c)
      : req_{std::move(req)},
        serverBiDiStreamFactory_{std::move(serverBiDiStreamFactory)},
        response_{std::move(response)},
        crc32c_{std::move(crc32c)} {}

  void operator()() noexcept {
    req_->sendBiDiReply(
        std::move(response_), std::move(serverBiDiStreamFactory_), crc32c_);
  }

 private:
  ResponseChannelRequest::UniquePtr req_;
  detail::ServerBiDiStreamFactory serverBiDiStreamFactory_;
  ResponsePayload response_;
  folly::Optional<uint32_t> crc32c_;
};

using ReplyInfo = std::variant<
    QueueReplyInfo,
    StreamReplyInfo,
    SinkConsumerReplyInfo,
    TilePromiseReplyInfo,
    BiDiStreamReplyInfo>;

/**
 * Used in EventBaseAtomicNotificationQueue to process each dequeued item
 */
class ReplyInfoConsumer {
 public:
  explicit ReplyInfoConsumer() {}
  void operator()(ReplyInfo&& info) noexcept {
    std::visit([](auto&& visitInfo) { visitInfo(); }, info);
  }
};

} // namespace apache::thrift
