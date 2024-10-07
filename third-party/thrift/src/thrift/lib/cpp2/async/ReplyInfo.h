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
#include <thrift/lib/cpp2/async/Sink.h>

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
      apache::thrift::detail::SinkConsumerImpl sinkConsumer,
      ResponsePayload&& response,
      folly::Optional<uint32_t> crc32c)
      : req_(std::move(req)),
        sinkConsumer_(std::move(sinkConsumer)),
        response_(std::move(response)),
        crc32c_(crc32c) {}

  void operator()() noexcept {
#if FOLLY_HAS_COROUTINES
    req_->sendSinkReply(
        std::move(response_), std::move(sinkConsumer_), crc32c_);
#endif
  }

  ResponseChannelRequest::UniquePtr req_;
  apache::thrift::detail::SinkConsumerImpl sinkConsumer_;
  ResponsePayload response_;
  folly::Optional<uint32_t> crc32c_;
};

using ReplyInfo =
    std::variant<QueueReplyInfo, StreamReplyInfo, SinkConsumerReplyInfo>;

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
