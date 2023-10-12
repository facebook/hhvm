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

#include <algorithm>
#include <utility>

#include <folly/portability/GTest.h>

#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/Singleton.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/test/Util.h>

using namespace apache::thrift::rocket;
using namespace apache::thrift::rocket::test;

namespace {
constexpr StreamId kTestStreamId{179};
constexpr folly::StringPiece kMetadata{"metadata"};
constexpr folly::StringPiece kData{"data"};

template <class Frame>
std::unique_ptr<folly::IOBuf> serialize(Frame frame) {
  Serializer writer;
  std::move(frame).serialize(writer);
  auto serializedFrameData = std::move(writer).move();
  serializedFrameData->coalesce();
  return serializedFrameData;
}

template <class Frame>
Frame serializeAndDeserialize(Frame frame) {
  auto serializedFrameData = serialize(std::move(frame));
  EXPECT_TRUE(isMaybeRocketFrame(*serializedFrameData));
  // Skip past frame length
  serializedFrameData->trimStart(Serializer::kBytesForFrameOrMetadataLength);
  return Frame(std::move(serializedFrameData));
}

template <class Frame>
Frame serializeAndDeserializeFragmented(Frame frame) {
  auto serializedFrameData = serialize(std::move(frame));
  folly::Optional<Frame> returnFrame;
  folly::io::Cursor cursor(serializedFrameData.get());
  bool hitSplitLogic = false;
  while (!cursor.isAtEnd()) {
    const auto currentFrameSize = readFrameOrMetadataSize(cursor);
    auto frameBuf = folly::IOBuf::createCombined(currentFrameSize);
    cursor.clone(*frameBuf, currentFrameSize);
    if (!returnFrame) {
      returnFrame.emplace(Frame(std::move(frameBuf)));
    } else {
      hitSplitLogic = true;
      PayloadFrame pf(std::move(frameBuf));
      returnFrame->payload().append(std::move(pf.payload()));
      if (!pf.hasFollows()) {
        break;
      }
    }
  }
  EXPECT_TRUE(returnFrame);
  EXPECT_TRUE(hitSplitLogic);
  return std::move(*returnFrame);
}

folly::IOBuf makeLargeIOBuf() {
  // Ensure IOBuf will be serialized across multiple fragment frames
  constexpr size_t kLargeIOBufSize = 0x2ffffff;

  folly::IOBuf buf(folly::IOBuf::CreateOp(), kLargeIOBufSize);
  EXPECT_GT(buf.tailroom(), kLargeIOBufSize);
  auto* const p = buf.writableTail();
  size_t i = 0;
  // Fill buffer with some non-trivial data
  std::for_each(p, p + kLargeIOBufSize, [&i](auto& c) {
    constexpr size_t kAPrime = 251;
    c = i++ % kAPrime;
  });
  buf.append(kLargeIOBufSize);
  return buf;
}

void validateMetadataAndData(const Payload& p) {
  auto dataAndMetadata = splitMetadataAndData(p);
  EXPECT_EQ(kMetadata, getRange(*dataAndMetadata.first));
  EXPECT_EQ(kData, getRange(*dataAndMetadata.second));
}
} // namespace

namespace apache {
namespace thrift {
namespace rocket {

TEST(FrameSerialization, SetupSanity) {
  SetupFrame frame(Payload::makeFromMetadataAndData(kMetadata, kData), false);

  auto validate = [](const SetupFrame& f) {
    // Resumption and lease flags are not currently supported
    EXPECT_FALSE(f.hasResumeIdentificationToken());
    EXPECT_FALSE(f.hasLease());
    validateMetadataAndData(f.payload());
  };

  validate(frame);
  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, RequestResponseSanity) {
  RequestResponseFrame frame(
      kTestStreamId, Payload::makeFromMetadataAndData(kMetadata, kData));

  auto validate = [](const RequestResponseFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_FALSE(f.hasFollows());
    validateMetadataAndData(f.payload());
  };

  validate(frame);
  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, RequestResponseNoHeadroomWriteForSharedBuf) {
  // Verify headroom write not attampted on shared IOBufs
  const std::string str = "12345678901234567890abcdef";
  auto dt = folly::IOBuf::wrapBuffer(str.data(), str.size());
  auto md = dt->clone();
  // share the same underlying buffer, but leave enough headroom for
  // frame header, if it was attempted to be written to the headroom
  md->trimStart(20);
  RequestResponseFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(std::move(md), std::move(dt)));

  auto serialBuf = std::move(frame).serialize();
  // deserialize
  serialBuf->coalesce();
  serialBuf->trimStart(Serializer::kBytesForFrameOrMetadataLength);
  RequestResponseFrame frame2(std::move(serialBuf));

  auto dataAndMetadata = splitMetadataAndData(frame2.payload());
  // Explicitly check copies in comparison, in case of erroneous write
  // to headroom original input data would be overwritten.
  EXPECT_EQ("abcdef", getRange(*dataAndMetadata.first));
  EXPECT_EQ("12345678901234567890abcdef", getRange(*dataAndMetadata.second));
}

TEST(FrameSerialization, RequestFnfSanity) {
  RequestFnfFrame frame(
      kTestStreamId, Payload::makeFromMetadataAndData(kMetadata, kData));

  auto validate = [](const RequestFnfFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_FALSE(f.hasFollows());
    validateMetadataAndData(f.payload());
  };

  validate(frame);
  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, RequestStreamSanity) {
  RequestStreamFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(kMetadata, kData),
      123456789 /* initialRequestN */);

  auto validate = [](const RequestStreamFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_FALSE(f.hasFollows());
    EXPECT_EQ(123456789, f.initialRequestN());
    validateMetadataAndData(f.payload());
  };

  validate(frame);
  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, RequestChannelSanity) {
  RequestChannelFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(kMetadata, kData),
      123456789 /* initialRequestN */);
  frame.setHasComplete(true);

  auto validate = [](const RequestChannelFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_FALSE(f.hasFollows());
    EXPECT_EQ(123456789, f.initialRequestN());
    validateMetadataAndData(f.payload());
    EXPECT_TRUE(f.hasComplete());
  };

  validate(frame);
  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, RequestNSanity) {
  RequestNFrame frame(kTestStreamId, std::numeric_limits<int32_t>::max());

  auto validate = [](const RequestNFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_EQ(std::numeric_limits<int32_t>::max(), f.requestN());
  };

  validate(frame);
  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, CancelSanity) {
  CancelFrame frame(kTestStreamId);

  auto validate = [](const CancelFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
  };

  validate(frame);
  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, PayloadSanity) {
  PayloadFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(kMetadata, kData),
      Flags().follows(true).complete(true).next(true));

  auto validate = [](const PayloadFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.hasFollows());
    EXPECT_TRUE(f.hasComplete());
    EXPECT_TRUE(f.hasNext());
    validateMetadataAndData(f.payload());
  };

  validate(frame);
  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, PayloadEmptyMetadataSanity) {
  auto validate = [](const PayloadFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.hasFollows());
    EXPECT_TRUE(f.hasComplete());
    EXPECT_TRUE(f.hasNext());
    EXPECT_FALSE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_EQ(kData, getRange(*dam.second));
  };

  // No metadata
  {
    PayloadFrame frame(
        kTestStreamId,
        Payload::makeFromData(kData),
        Flags().follows(true).complete(true).next(true));

    validate(frame);
    validate(serializeAndDeserialize(std::move(frame)));
  }

  // Empty metadata
  {
    PayloadFrame frame(
        kTestStreamId,
        Payload::makeFromMetadataAndData(
            folly::ByteRange{folly::StringPiece{""}}, kData),
        Flags().follows(true).complete(true).next(true));

    validate(frame);
    validate(serializeAndDeserialize(std::move(frame)));
  }
}

TEST(FrameSerialization, ErrorSanity) {
  constexpr folly::StringPiece kErrorMessage{"error_message"};

  ErrorFrame frame(
      kTestStreamId, ErrorCode::CANCELED, Payload::makeFromData(kErrorMessage));

  auto validate = [=](const ErrorFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_EQ(ErrorCode::CANCELED, f.errorCode());
    EXPECT_FALSE(f.payload().hasNonemptyMetadata());
    EXPECT_EQ(kErrorMessage, getRange(*f.payload().buffer()));
  };

  validate(frame);
  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, ErrorFromException) {
  constexpr folly::StringPiece kErrorMessage{"error_message"};

  RocketException rex(
      ErrorCode::CANCELED, folly::IOBuf::copyBuffer(kErrorMessage));
  ErrorFrame frame(kTestStreamId, std::move(rex));

  auto validate = [=](const ErrorFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_EQ(ErrorCode::CANCELED, f.errorCode());
    EXPECT_FALSE(f.payload().hasNonemptyMetadata());
    EXPECT_EQ(kErrorMessage, getRange(*f.payload().buffer()));
  };

  validate(frame);
  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, MetadataPushSanity) {
  constexpr folly::StringPiece kMeta{"transport_version"};

  auto makeMetadataPushFrame = [&] {
    std::unique_ptr<folly::IOBuf> buf(folly::IOBuf::create(6));
    buf->append(6);
    folly::io::RWPrivateCursor wcursor(buf.get());
    // write StreamId
    wcursor.writeBE<uint32_t>(0);
    // write frameType and flags
    wcursor.writeBE<uint16_t>(
        static_cast<uint8_t>(FrameType::METADATA_PUSH) << Flags::kBits |
        static_cast<uint16_t>(1 << 8));

    wcursor.insert(folly::IOBuf::copyBuffer(kMeta));
    buf->coalesce();
    return MetadataPushFrame(std::move(buf));
  };

  auto validate = [=](MetadataPushFrame&& f) {
    EXPECT_EQ(kMeta, getRange(*std::move(f).metadata()));
  };

  validate(makeMetadataPushFrame());
  validate(serializeAndDeserialize(makeMetadataPushFrame()));
}

TEST(FrameSerialization, KeepAliveSanity) {
  constexpr folly::StringPiece kKeepAliveData{"keep_alive_data"};

  auto makeKeepAliveFrame = [&] {
    return KeepAliveFrame(
        Flags().respond(true), folly::IOBuf::copyBuffer(kKeepAliveData));
  };

  auto validate = [=](KeepAliveFrame&& f) {
    EXPECT_TRUE(f.hasRespondFlag());
    EXPECT_EQ(kKeepAliveData, getRange(*std::move(f).data()));
  };

  validate(makeKeepAliveFrame());
  validate(serializeAndDeserialize(makeKeepAliveFrame()));
}

TEST(FrameSerialization, KeepAliveSanityEmptyPayload) {
  constexpr folly::StringPiece kKeepAliveData = folly::StringPiece();

  auto makeKeepAliveFrame = [&] {
    return KeepAliveFrame(
        Flags().respond(true), folly::IOBuf::copyBuffer(kKeepAliveData));
  };

  auto validate = [=](KeepAliveFrame&& f) {
    EXPECT_TRUE(f.hasRespondFlag());
    EXPECT_EQ(kKeepAliveData, getRange(*std::move(f).data()));
  };
  auto validateSerialized = [=](KeepAliveFrame&& f) {
    EXPECT_TRUE(f.hasRespondFlag());
    EXPECT_EQ(nullptr, std::move(f).data());
  };

  validate(makeKeepAliveFrame());
  validateSerialized(serializeAndDeserialize(makeKeepAliveFrame()));
}

TEST(FrameSerialization, PayloadLargeMetadata) {
  const auto metadata = makeLargeIOBuf();

  PayloadFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(
          metadata.clone(), folly::IOBuf::copyBuffer(kData)),
      Flags().complete(true).next(true));

  auto validate = [=](const PayloadFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_TRUE(folly::IOBufEqualTo()(metadata, *dam.first));
    EXPECT_EQ(kData, getRange(*dam.second));
  };

  validate(frame);
  validate(serializeAndDeserializeFragmented(std::move(frame)));
}

TEST(FrameSerialization, PayloadLargeData) {
  const auto data = makeLargeIOBuf();

  PayloadFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(
          folly::IOBuf::copyBuffer(kMetadata), data.clone()),
      Flags().complete(true).next(true));

  auto validate = [=](const PayloadFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_EQ(kMetadata, getRange(*dam.first));
    EXPECT_TRUE(folly::IOBufEqualTo()(data, *dam.second));
  };

  validate(frame);
  validate(serializeAndDeserializeFragmented(std::move(frame)));
}

TEST(FrameSerialization, RequestResponseLargeMetadata) {
  const auto metadata = makeLargeIOBuf();

  RequestResponseFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(
          metadata.clone(), folly::IOBuf::copyBuffer(kData)));

  auto validate = [=](const RequestResponseFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_TRUE(folly::IOBufEqualTo()(metadata, *dam.first));
    EXPECT_EQ(kData, getRange(*dam.second));
  };

  validate(frame);
  validate(serializeAndDeserializeFragmented(std::move(frame)));
}

TEST(FrameSerialization, RequestResponseLargeData) {
  const auto data = makeLargeIOBuf();

  RequestResponseFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(
          folly::IOBuf::copyBuffer(kMetadata), data.clone()));

  auto validate = [=](const RequestResponseFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_EQ(kMetadata, getRange(*dam.first));
    EXPECT_TRUE(folly::IOBufEqualTo()(data, *dam.second));
  };

  validate(frame);
  validate(serializeAndDeserializeFragmented(std::move(frame)));
}

TEST(FrameSerialization, RequestFnfLargeMetadata) {
  const auto metadata = makeLargeIOBuf();

  RequestFnfFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(
          metadata.clone(), folly::IOBuf::copyBuffer(kData)));

  auto validate = [=](const RequestFnfFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_TRUE(folly::IOBufEqualTo()(metadata, *dam.first));
    EXPECT_EQ(kData, getRange(*dam.second));
  };

  validate(frame);
  validate(serializeAndDeserializeFragmented(std::move(frame)));
}

TEST(FrameSerialization, RequestFnfLargeData) {
  const auto data = makeLargeIOBuf();

  RequestFnfFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(
          folly::IOBuf::copyBuffer(kMetadata), data.clone()));

  auto validate = [=](const RequestFnfFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_EQ(kMetadata, getRange(*dam.first));
    EXPECT_TRUE(folly::IOBufEqualTo()(data, *dam.second));
  };

  validate(frame);
  validate(serializeAndDeserializeFragmented(std::move(frame)));
}

TEST(FrameSerialization, RequestStreamLargeMetadata) {
  const auto metadata = makeLargeIOBuf();

  RequestStreamFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(
          metadata.clone(), folly::IOBuf::copyBuffer(kData)),
      123 /* initialRequestN */);

  auto validate = [=](const RequestStreamFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_TRUE(folly::IOBufEqualTo()(metadata, *dam.first));
    EXPECT_EQ(kData, getRange(*dam.second));
  };

  validate(frame);
  validate(serializeAndDeserializeFragmented(std::move(frame)));
}

TEST(FrameSerialization, RequestStreamLargeData) {
  const auto data = makeLargeIOBuf();

  RequestStreamFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(
          folly::IOBuf::copyBuffer(kMetadata), data.clone()),
      123 /* initialRequestN */);

  auto validate = [=](const RequestStreamFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_EQ(kMetadata, getRange(*dam.first));
    EXPECT_TRUE(folly::IOBufEqualTo()(data, *dam.second));
  };

  validate(frame);
  validate(serializeAndDeserializeFragmented(std::move(frame)));
}

TEST(FrameSerialization, RequestChannelLargeMetadata) {
  const auto metadata = makeLargeIOBuf();

  RequestChannelFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(
          metadata.clone(), folly::IOBuf::copyBuffer(kData)),
      123 /* initialRequestN */);
  frame.setHasComplete(true);

  auto validate = [=](const RequestChannelFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_TRUE(folly::IOBufEqualTo()(metadata, *dam.first));
    EXPECT_EQ(kData, getRange(*dam.second));
    EXPECT_TRUE(f.hasComplete());
  };

  validate(frame);
  validate(serializeAndDeserializeFragmented(std::move(frame)));
}

TEST(FrameSerialization, RequestChannelLargeData) {
  const auto data = makeLargeIOBuf();

  RequestChannelFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(
          folly::IOBuf::copyBuffer(kMetadata), data.clone()),
      123 /* initialRequestN */);
  frame.setHasComplete(true);

  auto validate = [=](const RequestChannelFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_TRUE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_EQ(kMetadata, getRange(*dam.first));
    EXPECT_TRUE(folly::IOBufEqualTo()(data, *dam.second));
    EXPECT_TRUE(f.hasComplete());
  };

  validate(frame);
  validate(serializeAndDeserializeFragmented(std::move(frame)));
}

TEST(FrameSerialization, PayloadFrameSerializeAPI) {
  auto validate = [](std::unique_ptr<folly::IOBuf> md,
                     std::unique_ptr<folly::IOBuf> data) {
    folly::IOBufEqualTo eq;
    PayloadFrame frame1(
        kTestStreamId,
        Payload::makeFromMetadataAndData(md->clone(), data->clone()),
        Flags().follows(true).complete(true).next(true));
    PayloadFrame frame2(
        kTestStreamId,
        Payload::makeFromMetadataAndData(md->clone(), data->clone()),
        Flags().follows(true).complete(true).next(true));
    auto altApi = std::move(frame1).serialize();
    Serializer writer;
    std::move(frame2).serialize(writer);
    auto regApi = std::move(writer).move();
    EXPECT_TRUE(eq(altApi, regApi));
  };

  // Base, No headroom in metadata.
  validate(
      folly::IOBuf::wrapBuffer(kMetadata), folly::IOBuf::wrapBuffer(kData));
  // Base, with headroom.
  validate(
      folly::IOBuf::wrapBuffer(kMetadata)->cloneCoalescedWithHeadroomTailroom(
          16, 256),
      folly::IOBuf::wrapBuffer(kData));

  // Bigger metadata, no headroom.
  constexpr size_t kBigish = 8 << 10;
  auto bigIOB = folly::IOBuf::create(kBigish);
  bigIOB->append(kBigish);
  validate(std::move(bigIOB), folly::IOBuf::wrapBuffer(kData));

  // Bigger metadata, with headroom.
  auto bigIOBWithHeadroom = folly::IOBuf::create(kBigish);
  bigIOBWithHeadroom->advance(16);
  bigIOBWithHeadroom->append(kBigish - 16);
  validate(std::move(bigIOBWithHeadroom), folly::IOBuf::wrapBuffer(kData));

  // Empty metadata
  validate(std::make_unique<folly::IOBuf>(), folly::IOBuf::wrapBuffer(kData));
}

TEST(FrameSerialization, ExtUnknownSanity) {
  ExtFrame frame(
      kTestStreamId,
      Payload::makeFromMetadataAndData(kMetadata, kData),
      Flags().ignore(true),
      static_cast<ExtFrameType>(42));

  auto validate = [](const ExtFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_EQ(ExtFrameType::UNKNOWN, f.extFrameType());
    EXPECT_TRUE(f.hasIgnore());
    validateMetadataAndData(f.payload());
  };

  validate(serializeAndDeserialize(std::move(frame)));
}

TEST(FrameSerialization, ExtEmptyMetadataSanity) {
  auto validate = [](const ExtFrame& f) {
    EXPECT_EQ(kTestStreamId, f.streamId());
    EXPECT_EQ(ExtFrameType::UNKNOWN, f.extFrameType());
    EXPECT_TRUE(f.hasIgnore());
    EXPECT_FALSE(f.payload().hasNonemptyMetadata());
    auto dam = splitMetadataAndData(f.payload());
    EXPECT_EQ(kData, getRange(*dam.second));
  };

  // No metadata
  {
    ExtFrame frame(
        kTestStreamId,
        Payload::makeFromData(kData),
        Flags().ignore(true),
        ExtFrameType::UNKNOWN);

    validate(frame);
    validate(serializeAndDeserialize(std::move(frame)));
  }

  // Empty metadata
  {
    ExtFrame frame(
        kTestStreamId,
        Payload::makeFromMetadataAndData(
            folly::ByteRange{folly::StringPiece{""}}, kData),
        Flags().ignore(true),
        ExtFrameType::UNKNOWN);

    validate(frame);
    validate(serializeAndDeserialize(std::move(frame)));
  }
}

} // namespace rocket
} // namespace thrift
} // namespace apache

// TODO: Needed to make xplat test happy, we can get rid of this once we
// remove fix_cpp_fragmentation flag.
int main(int argc, char** argv) {
  // Enable glog logging to stderr by default.
  FLAGS_logtostderr = true;

  ::testing::InitGoogleTest(&argc, argv);
  folly::SingletonVault::singleton()->registrationComplete();

  return RUN_ALL_TESTS();
}
