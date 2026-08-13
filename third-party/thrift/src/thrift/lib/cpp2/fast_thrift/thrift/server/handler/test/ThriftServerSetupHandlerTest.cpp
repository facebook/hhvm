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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerSetupHandler.h>

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/SetupResponseBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ConnectionPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

using channel_pipeline::erase_and_box;
using channel_pipeline::Result;
using channel_pipeline::TypeErasedBox;

class FakeContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    forwarded.push_back(std::move(msg));
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    written.push_back(std::move(msg));
    return writeResult;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception = std::move(e);
  }

  std::vector<TypeErasedBox> forwarded;
  std::vector<TypeErasedBox> written;
  folly::exception_wrapper exception;
  Result writeResult{Result::Success};
};

ThriftServerRequestMessage makeSetupMessage(
    int32_t clientMinVersion,
    int32_t clientMaxVersion,
    std::optional<SetupRejection> reject = std::nullopt) {
  auto setup = std::make_unique<ConnectionSetupData>();
  setup->clientSetup.minVersion() = clientMinVersion;
  setup->clientSetup.maxVersion() = clientMaxVersion;
  setup->reject = std::move(reject);

  ThriftServerRequestMessage msg;
  msg.payload = ThriftConnectionSetupPayload{.setup = std::move(setup)};
  return msg;
}

apache::thrift::SetupResponse decodeSetupResponse(folly::IOBuf* buffer) {
  apache::thrift::CompactProtocolReader reader;
  reader.setInput(buffer);
  apache::thrift::ServerPushMetadata push;
  push.read(&reader);
  EXPECT_EQ(
      push.getType(), apache::thrift::ServerPushMetadata::Type::setupResponse);
  return *push.setupResponse();
}

} // namespace

// A client whose advertised range overlaps the server's is answered with a
// SETUP response carrying the negotiated version. It goes out structured, so
// handlers downstream on the write path can still add to it.
TEST(ThriftServerSetupHandlerTest, OverlappingVersionRangeIsAnswered) {
  ThriftServerSetupHandler<FakeContext> handler;
  FakeContext ctx;

  EXPECT_EQ(
      handler.onRead(
          ctx,
          erase_and_box(
              makeSetupMessage(kMinNegotiableVersion, kMaxNegotiableVersion))),
      Result::Success);

  EXPECT_TRUE(ctx.forwarded.empty()) << "setup terminates here";
  ASSERT_EQ(ctx.written.size(), 1);
  auto& response = ctx.written.front().get<ThriftServerResponseMessage>();
  ASSERT_TRUE(response.payload.is<ThriftSetupResponsePayload>());
  const auto& setupResponse =
      response.payload.get<ThriftSetupResponsePayload>().response;
  ASSERT_NE(setupResponse, nullptr);
  EXPECT_EQ(setupResponse->version().value_or(-1), kMaxNegotiableVersion);
  // Handler-owned fields are left for the write path to stamp.
  EXPECT_FALSE(setupResponse->securityPolicy().has_value());
}

// A handler upstream that set `reject` is answered with its reason, without
// having written anything itself.
TEST(ThriftServerSetupHandlerTest, UpstreamRejectionBecomesTheAnswer) {
  ThriftServerSetupHandler<FakeContext> handler;
  FakeContext ctx;

  EXPECT_EQ(
      handler.onRead(
          ctx,
          erase_and_box(makeSetupMessage(
              kMinNegotiableVersion,
              kMaxNegotiableVersion,
              SetupRejection{
                  .code = apache::thrift::fast_thrift::frame::ErrorCode::
                      REJECTED_SETUP,
                  .reason = "not welcome here"}))),
      Result::Error);

  ASSERT_EQ(ctx.written.size(), 1);
  auto& response = ctx.written.front().get<ThriftServerResponseMessage>();
  ASSERT_TRUE(response.payload.is<ThriftSetupRejectionPayload>());
  const auto& rejection = response.payload.get<ThriftSetupRejectionPayload>();
  EXPECT_EQ(
      rejection.errorCode,
      static_cast<uint32_t>(
          apache::thrift::fast_thrift::frame::ErrorCode::REJECTED_SETUP));
  EXPECT_EQ(rejection.reason->to<std::string>(), "not welcome here");
}

// No overlap with the server's range is a refusal: a rejection payload out,
// and non-Success so the exchange stops here.
TEST(ThriftServerSetupHandlerTest, VersionRangeOutsideServerRangeIsRefused) {
  ThriftServerSetupHandler<FakeContext> handler;
  FakeContext ctx;

  EXPECT_EQ(
      handler.onRead(
          ctx,
          erase_and_box(makeSetupMessage(
              kMaxNegotiableVersion + 1, kMaxNegotiableVersion + 1))),
      Result::Error);

  EXPECT_TRUE(ctx.forwarded.empty());
  ASSERT_EQ(ctx.written.size(), 1);
  auto& response = ctx.written.front().get<ThriftServerResponseMessage>();
  ASSERT_TRUE(response.payload.is<ThriftSetupRejectionPayload>());
  EXPECT_EQ(
      response.payload.get<ThriftSetupRejectionPayload>().errorCode,
      static_cast<uint32_t>(
          apache::thrift::fast_thrift::frame::ErrorCode::INVALID_SETUP));
}

// The point of answering with a structured response: a handler on the write
// path stamps the field it owns, and both its contribution and the server's
// negotiated fields reach the wire together.
TEST(ThriftServerSetupHandlerTest, WritePathContributionReachesTheWire) {
  ThriftServerSetupHandler<FakeContext> handler;
  FakeContext ctx;

  ASSERT_EQ(
      handler.onRead(
          ctx,
          erase_and_box(
              makeSetupMessage(kMinNegotiableVersion, kMaxNegotiableVersion))),
      Result::Success);
  ASSERT_EQ(ctx.written.size(), 1);

  // Stand in for a handler downstream on the write path.
  auto response =
      std::move(ctx.written.front().get<ThriftServerResponseMessage>().payload);
  apache::thrift::SecurityPolicy policy;
  policy.authorization() = apache::thrift::SecurityPolicyStatus::ENABLED;
  response.get<ThriftSetupResponsePayload>().response->securityPolicy() =
      std::move(policy);

  auto frame = std::move(response).toRocketFrame(
      rocket::server::MetadataProtocol::COMPACT);
  ASSERT_NE(frame.metadata, nullptr);
  const auto decoded = decodeSetupResponse(frame.metadata.get());
  EXPECT_EQ(decoded.version().value_or(-1), kMaxNegotiableVersion);
  ASSERT_TRUE(decoded.securityPolicy().has_value());
  EXPECT_EQ(
      *decoded.securityPolicy()->authorization(),
      apache::thrift::SecurityPolicyStatus::ENABLED);
}

// The transport reports Backpressure for any write still in flight, which is
// every response. It must not surface as a read result, or reads would pause
// behind each one.
TEST(ThriftServerSetupHandlerTest, InFlightResponseWriteDoesNotPauseReads) {
  ThriftServerSetupHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.writeResult = Result::Backpressure;

  EXPECT_EQ(
      handler.onRead(
          ctx,
          erase_and_box(
              makeSetupMessage(kMinNegotiableVersion, kMaxNegotiableVersion))),
      Result::Success);

  ASSERT_EQ(ctx.written.size(), 1);
  EXPECT_TRUE(ctx.written.front()
                  .get<ThriftServerResponseMessage>()
                  .payload.is<ThriftSetupResponsePayload>());
}

// A response that cannot be written leaves the client waiting on an answer
// that will never arrive, so the exchange fails rather than reporting success.
TEST(ThriftServerSetupHandlerTest, UndeliverableResponseFailsSetup) {
  ThriftServerSetupHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.writeResult = Result::Error;

  EXPECT_EQ(
      handler.onRead(
          ctx,
          erase_and_box(
              makeSetupMessage(kMinNegotiableVersion, kMaxNegotiableVersion))),
      Result::Error);
}

} // namespace apache::thrift::fast_thrift::thrift
