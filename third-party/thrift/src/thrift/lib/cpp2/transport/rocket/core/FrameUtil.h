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

#include <folly/SingletonThreadLocal.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

namespace apache::thrift::rocket::frame_util {

template <typename Frame>
struct FrameTraits;

namespace detail {

using FrameVariant = std::variant<
    std::monostate,
    SetupFrame,
    RequestResponseFrame,
    RequestFnfFrame,
    RequestStreamFrame,
    RequestChannelFrame,
    RequestNFrame,
    CancelFrame,
    PayloadFrame,
    MetadataPushFrame,
    KeepAliveFrame,
    ErrorFrame,
    ExtFrame>;

/**
 * FrameTraits is a type trait that provides information about a frame type.
 * It defines:
 * - The frame type enum value
 * - Whether the frame is a stream frame
 * - Whether the frame is a stream zero frame
 * - Methods to construct, destruct, serialize, and deserialize frames
 *
 * This trait is used to provide a uniform interface for working with different
 * frame types in the Rocket RPC protocol.
 */
template <
    apache::thrift::rocket::FrameType FrameTypeEnum,
    typename Framing,
    bool IsStream,
    bool IsStreamZero>
struct FrameTraitsImpl {
  using Frame = Framing;
  static constexpr apache::thrift::rocket::FrameType type = FrameTypeEnum;
  static constexpr apache::thrift::rocket::FrameType getType() { return type; }
  static constexpr bool isStreamFrame() { return IsStream; }
  static constexpr bool isStreamZeroFrame() { return IsStreamZero; }
  static void construct(FrameVariant& u, Framing&& f) {
    u.emplace<Frame>(std::move(f));
  }
  static void destruct(FrameVariant& u) {
    if (u.index() != std::variant_npos && std::holds_alternative<Frame>(u)) {
      std::get<Frame>(u).~Framing();
    }
  }
  static std::unique_ptr<folly::IOBuf> serialize(Framing& f) {
    auto& serializer = folly::SingletonThreadLocal<Serializer>::get();
    std::move(f).serialize(serializer);
    return serializer.moveAndReset();
  }
  static FOLLY_ALWAYS_INLINE std::unique_ptr<folly::IOBuf> serialize(
      FrameVariant& u) {
    DCHECK(u.index() != std::variant_npos && std::holds_alternative<Frame>(u))
        << "Variant is valueless or has wrong type";
    return serialize(std::get<Frame>(u));
  }
  static Framing deserialize(std::unique_ptr<folly::IOBuf> frame) {
    return Framing(std::move(frame));
  }
};

template <FrameType type>
struct FrameTypeToTraits;

/**
 * FrameTypeToTraits is a template struct that maps a FrameType enum value
 * to the corresponding FrameTraits specialization. This allows code to look up
 * the appropriate traits for a frame type at compile time.
 *
 * For example, FrameTypeToTraits<FrameType::SETUP>::Traits would give you
 * the traits for SetupFrame.
 *
 * This is used in conjunction with the MAP_FRAME_TYPE_TO_TRAITS macro below
 * which creates the specializations for each frame type.
 */
#define THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameTypeEnum, Framing) \
  template <>                                                  \
  struct FrameTypeToTraits<FrameTypeEnum> {                    \
    using Traits = FrameTraits<Framing>;                       \
  };

THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::SETUP, SetupFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(
    FrameType::REQUEST_RESPONSE, RequestResponseFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::REQUEST_FNF, RequestFnfFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::REQUEST_STREAM, RequestStreamFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::REQUEST_CHANNEL, RequestChannelFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::REQUEST_N, RequestNFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::CANCEL, CancelFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::PAYLOAD, PayloadFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::METADATA_PUSH, MetadataPushFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::KEEPALIVE, KeepAliveFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::ERROR, ErrorFrame)
THRIFT_MAP_FRAME_TYPE_TO_TRAIT(FrameType::EXT, ExtFrame)

#undef THRIFT_MAP_FRAME_TYPE_TO_TRAIT

} // namespace detail

// This macro defines a specialization of the FrameTraits struct for a given
// frame type. It takes four parameters:
// - FrameTypeEnum: The enum value of the frame type.
// - Framing: The C++ type of the frame.
// - IsStream: A boolean indicating whether this frame is a stream frame.
// - IsStreamZero: A boolean indicating whether this frame is a stream zero
//   frame.
#define THRIFT_DEFINE_FRAME_TRAIT(                                            \
    FrameTypeEnum, Framing, IsStream, IsStreamZero)                           \
  template <>                                                                 \
  struct FrameTraits<Framing>                                                 \
      : detail::                                                              \
            FrameTraitsImpl<FrameTypeEnum, Framing, IsStream, IsStreamZero> { \
  };

THRIFT_DEFINE_FRAME_TRAIT(FrameType::SETUP, SetupFrame, false, false)
THRIFT_DEFINE_FRAME_TRAIT(
    FrameType::REQUEST_RESPONSE, RequestResponseFrame, true, false)
THRIFT_DEFINE_FRAME_TRAIT(FrameType::REQUEST_FNF, RequestFnfFrame, true, false)
THRIFT_DEFINE_FRAME_TRAIT(
    FrameType::REQUEST_STREAM, RequestStreamFrame, true, false)
THRIFT_DEFINE_FRAME_TRAIT(
    FrameType::REQUEST_CHANNEL, RequestChannelFrame, true, false)
THRIFT_DEFINE_FRAME_TRAIT(FrameType::REQUEST_N, RequestNFrame, true, false)
THRIFT_DEFINE_FRAME_TRAIT(FrameType::CANCEL, CancelFrame, true, false)
THRIFT_DEFINE_FRAME_TRAIT(FrameType::PAYLOAD, PayloadFrame, true, false)
THRIFT_DEFINE_FRAME_TRAIT(
    FrameType::METADATA_PUSH, MetadataPushFrame, false, true)
THRIFT_DEFINE_FRAME_TRAIT(FrameType::KEEPALIVE, KeepAliveFrame, false, true)
THRIFT_DEFINE_FRAME_TRAIT(FrameType::ERROR, ErrorFrame, true, false)
THRIFT_DEFINE_FRAME_TRAIT(FrameType::EXT, ExtFrame, false, false)

#undef THRIFT_DEFINE_FRAME_TRAITS

template <typename Frame>
std::unique_ptr<folly::IOBuf> serialize(Frame& f) {
  return FrameTraits<Frame>::serialize(f);
}

struct FrameContainer {
  using FrameVariant = detail::FrameVariant;
  using DestructorFn = void (*)(FrameVariant&);
  using SerializerFn = std::unique_ptr<folly::IOBuf> (*)(FrameVariant&);

  FrameType type;
  FrameVariant frame;
  DestructorFn destructor;
  SerializerFn serializer{};

  FrameContainer() : type(FrameType::RESERVED) {
    destructor = [](FrameVariant&) {};
  }

  FrameContainer(FrameContainer&& other) noexcept
      : type(other.type),
        frame(std::move(other.frame)),
        destructor(other.destructor),
        serializer(other.serializer) {
    other.type = FrameType::RESERVED;
    other.destructor = [](FrameVariant&) {};
    other.serializer = nullptr;
  }

  FrameContainer& operator=(FrameContainer&& other) noexcept {
    if (this != &other) {
      destructor(frame);

      type = other.type;
      frame = std::move(other.frame);
      destructor = other.destructor;
      serializer = other.serializer;

      other.type = FrameType::RESERVED;
      other.destructor = [](FrameVariant&) {};
      other.serializer = nullptr;
    }
    return *this;
  }

  FrameContainer(const FrameContainer&) = delete;
  FrameContainer& operator=(const FrameContainer&) = delete;

  bool empty() { return type == FrameType::RESERVED; }

  template <typename Frame>
  FrameContainer(Frame&& f) {
    type = FrameTraits<Frame>::type;
    FrameTraits<Frame>::construct(frame, std::move(f));
    destructor = FrameTraits<Frame>::destruct;
    serializer = FrameTraits<Frame>::serialize;
  }

  template <typename T>
  struct always_false : std::false_type {};

  ~FrameContainer() { destructor(frame); }

  std::unique_ptr<folly::IOBuf> serialize() {
    CHECK(serializer) << "Invalid frame type for serialization";
    return serializer(frame);
  }
};

template <typename Frame>
constexpr bool isStreamFrame() {
  return FrameTraits<Frame>::isStreamFrame();
}

template <typename Frame>
constexpr bool isStreamZeroFrame() {
  return FrameTraits<Frame>::isStreamZeroFrame();
}

template <FrameType type>
constexpr uint8_t rawFrameType() {
  auto constexpr raw = static_cast<uint8_t>(type);
  return raw;
}

template <FrameType type>
using FrameTypeToTraits_t = typename detail::FrameTypeToTraits<type>::Traits;

} // namespace apache::thrift::rocket::frame_util
