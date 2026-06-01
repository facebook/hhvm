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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/RpcKindMapping.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

TEST(RpcKindMappingTest, SingleRequestSingleResponse) {
  EXPECT_EQ(
      rpcKindToFrameType(
          apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
}

TEST(RpcKindMappingTest, SingleRequestNoResponse) {
  EXPECT_EQ(
      rpcKindToFrameType(apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_FNF);
}

TEST(RpcKindMappingTest, SingleRequestStreamingResponse) {
  EXPECT_EQ(
      rpcKindToFrameType(
          apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM);
}

TEST(RpcKindMappingTest, UnsupportedKindsReturnReserved) {
  EXPECT_EQ(
      rpcKindToFrameType(apache::thrift::RpcKind::SINK),
      apache::thrift::fast_thrift::frame::FrameType::RESERVED);
  EXPECT_EQ(
      rpcKindToFrameType(apache::thrift::RpcKind::BIDIRECTIONAL_STREAM),
      apache::thrift::fast_thrift::frame::FrameType::RESERVED);
}

} // namespace apache::thrift::fast_thrift::thrift
