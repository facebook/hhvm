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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerRequestHeadersHandler.h>

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftRequestContext.h>
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

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception = std::move(e);
  }

  std::vector<TypeErasedBox> forwarded;
  folly::exception_wrapper exception;
};

// Builds a request-response message with a fresh (empty) request context, as
// the upstream ThriftServerRequestContextHandler would stamp it.
ThriftServerRequestMessage makeRequest(
    uint32_t streamId,
    std::unique_ptr<apache::thrift::RequestRpcMetadata> metadata) {
  ThriftServerRequestMessage req;
  req.streamId = streamId;
  req.requestContext = std::make_unique<ThriftRequestContext>();
  req.payload = ThriftServerInboundPayloadVariant{ThriftRequestResponsePayload{
      .data = folly::IOBuf::copyBuffer("body"),
      .metadata = std::move(metadata)}};
  return req;
}

std::unique_ptr<apache::thrift::RequestRpcMetadata> makeMetadata() {
  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  metadata->name() = "Service.method";
  return metadata;
}

} // namespace

TEST(ThriftServerRequestHeadersHandlerTest, StampsHeadersOntoRequestContext) {
  auto metadata = makeMetadata();
  auto& headers = metadata->otherMetadata().ensure();
  headers["kcb_identity"] = "svc:foo";
  headers["client_identifier"] = "client-123";

  ThriftServerRequestHeadersHandler<FakeContext> handler;
  FakeContext ctx;

  EXPECT_EQ(
      handler.onRead(
          ctx,
          erase_and_box(makeRequest(/*streamId=*/42, std::move(metadata)))),
      Result::Success);

  ASSERT_EQ(ctx.forwarded.size(), 1);
  auto& forwarded = ctx.forwarded.front().get<ThriftServerRequestMessage>();
  EXPECT_EQ(forwarded.streamId, 42);
  ASSERT_NE(forwarded.requestContext, nullptr);

  const auto& stamped = forwarded.requestContext->getHeaders();
  EXPECT_EQ(stamped.size(), 2);

  // getHeader accepts a string_view (heterogeneous lookup, no temporary).
  const std::string* kcb = forwarded.requestContext->getHeader("kcb_identity");
  ASSERT_NE(kcb, nullptr);
  EXPECT_EQ(*kcb, "svc:foo");
  const std::string* cid =
      forwarded.requestContext->getHeader("client_identifier");
  ASSERT_NE(cid, nullptr);
  EXPECT_EQ(*cid, "client-123");
}

TEST(ThriftServerRequestHeadersHandlerTest, MissingHeaderReturnsNullptr) {
  auto metadata = makeMetadata();
  metadata->otherMetadata().ensure()["present"] = "yes";

  ThriftServerRequestHeadersHandler<FakeContext> handler;
  FakeContext ctx;

  ASSERT_EQ(
      handler.onRead(ctx, erase_and_box(makeRequest(1, std::move(metadata)))),
      Result::Success);

  auto& forwarded = ctx.forwarded.front().get<ThriftServerRequestMessage>();
  EXPECT_EQ(forwarded.requestContext->getHeader("absent"), nullptr);
}

TEST(ThriftServerRequestHeadersHandlerTest, NoOtherMetadataLeavesHeadersEmpty) {
  // Metadata present but carrying no custom headers.
  ThriftServerRequestHeadersHandler<FakeContext> handler;
  FakeContext ctx;

  ASSERT_EQ(
      handler.onRead(ctx, erase_and_box(makeRequest(7, makeMetadata()))),
      Result::Success);

  auto& forwarded = ctx.forwarded.front().get<ThriftServerRequestMessage>();
  ASSERT_NE(forwarded.requestContext, nullptr);
  EXPECT_TRUE(forwarded.requestContext->getHeaders().empty());
}

TEST(ThriftServerRequestHeadersHandlerTest, NullMetadataLeavesHeadersEmpty) {
  // Defensive: a payload with no metadata must not crash the handler.
  ThriftServerRequestHeadersHandler<FakeContext> handler;
  FakeContext ctx;

  ASSERT_EQ(
      handler.onRead(ctx, erase_and_box(makeRequest(9, /*metadata=*/nullptr))),
      Result::Success);

  auto& forwarded = ctx.forwarded.front().get<ThriftServerRequestMessage>();
  ASSERT_NE(forwarded.requestContext, nullptr);
  EXPECT_TRUE(forwarded.requestContext->getHeaders().empty());
}

TEST(ThriftServerRequestHeadersHandlerTest, ForwardsExceptions) {
  ThriftServerRequestHeadersHandler<FakeContext> handler;
  FakeContext ctx;

  handler.onException(
      ctx, folly::make_exception_wrapper<std::runtime_error>("boom"));

  ASSERT_TRUE(ctx.exception);
  EXPECT_NE(ctx.exception.what().toStdString().find("boom"), std::string::npos);
}

} // namespace apache::thrift::fast_thrift::thrift
