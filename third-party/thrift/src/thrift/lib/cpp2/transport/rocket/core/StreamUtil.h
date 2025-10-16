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

#include <folly/Traits.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/SafeAssert.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

namespace apache::thrift::rocket::stream_util {

// RSocket Interaction Stream Types - mapped to FrameType id for convince
enum class RequestKind : uint8_t {
  REQUEST_RESPONSE = static_cast<uint8_t>(FrameType::REQUEST_RESPONSE),
  REQUEST_FNF = static_cast<uint8_t>(FrameType::REQUEST_FNF),
  REQUEST_STREAM = static_cast<uint8_t>(FrameType::REQUEST_STREAM),
  REQUEST_CHANNEL = static_cast<uint8_t>(FrameType::REQUEST_CHANNEL),
};

template <auto Kind>
constexpr bool isRequestKind() {
  constexpr uint8_t rawValue = static_cast<uint8_t>(Kind);
  return rawValue == static_cast<uint8_t>(RequestKind::REQUEST_RESPONSE) || //
      rawValue == static_cast<uint8_t>(RequestKind::REQUEST_FNF) || //
      rawValue == static_cast<uint8_t>(RequestKind::REQUEST_STREAM) ||
      rawValue == static_cast<uint8_t>(RequestKind::REQUEST_CHANNEL);
}

// RSocket frames used in a stream
enum class StreamFrameType : uint8_t {
  REQUEST_RESPONSE = static_cast<uint8_t>(FrameType::REQUEST_RESPONSE),
  REQUEST_FNF = static_cast<uint8_t>(FrameType::REQUEST_FNF),
  REQUEST_STREAM = static_cast<uint8_t>(FrameType::REQUEST_STREAM),
  REQUEST_CHANNEL = static_cast<uint8_t>(FrameType::REQUEST_CHANNEL),
  REQUEST_N = static_cast<uint8_t>(FrameType::REQUEST_N),
  CANCEL = static_cast<uint8_t>(FrameType::CANCEL),
  PAYLOAD = static_cast<uint8_t>(FrameType::PAYLOAD),
  ERROR = static_cast<uint8_t>(FrameType::ERROR),
};

template <auto Type>
constexpr bool isStreamFrameType() {
  constexpr uint8_t rawValue = static_cast<uint8_t>(Type);
  return rawValue ==
      static_cast<uint8_t>(StreamFrameType::REQUEST_RESPONSE) || //
      rawValue == static_cast<uint8_t>(StreamFrameType::REQUEST_FNF) || //
      rawValue == static_cast<uint8_t>(StreamFrameType::REQUEST_STREAM) || //
      rawValue == static_cast<uint8_t>(StreamFrameType::REQUEST_CHANNEL) || //
      rawValue == static_cast<uint8_t>(StreamFrameType::REQUEST_N) || //
      rawValue == static_cast<uint8_t>(StreamFrameType::CANCEL) || //
      rawValue == static_cast<uint8_t>(StreamFrameType::PAYLOAD) || //
      rawValue == static_cast<uint8_t>(StreamFrameType::ERROR);
}

// Frames that must be on stream zero
enum class StreamZeroFrameType : uint8_t {
  KEEPALIVE = static_cast<uint8_t>(FrameType::KEEPALIVE),
  METADATA_PUSH = static_cast<uint8_t>(FrameType::METADATA_PUSH),
};

template <auto Type>
constexpr bool isStreamZeroFrameType() {
  constexpr uint8_t rawValue = static_cast<uint8_t>(Type);
  return rawValue == static_cast<uint8_t>(StreamZeroFrameType::KEEPALIVE) || //
      rawValue == static_cast<uint8_t>(StreamZeroFrameType::METADATA_PUSH);
}

template <auto Type>
constexpr bool isKeepAliveFrameType() {
  constexpr uint8_t rawValue = static_cast<uint8_t>(Type);
  return rawValue == static_cast<uint8_t>(StreamZeroFrameType::KEEPALIVE);
}

template <auto Type>
constexpr bool isMetadataPushFrameType() {
  constexpr uint8_t rawValue = static_cast<uint8_t>(Type);
  return rawValue == static_cast<uint8_t>(StreamZeroFrameType::METADATA_PUSH);
}

constexpr StreamId kStreamIdZero = StreamId(0);

template <typename Frame>
bool isStreamIdZero(const Frame& frame) {
  return frame.streamId() == kStreamIdZero;
}

template <typename Frame>
bool isStreamIdNonZero(const Frame& frame) {
  return frame.streamId() != kStreamIdZero;
}

template <typename Frame>
uint32_t rawStreamId(const Frame& frame) {
  return static_cast<uint32_t>(frame.streamId());
}

} // namespace apache::thrift::rocket::stream_util
