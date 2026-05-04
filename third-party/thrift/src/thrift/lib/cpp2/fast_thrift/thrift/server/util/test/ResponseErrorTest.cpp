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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseError.h>

namespace apache::thrift::fast_thrift::thrift {

// INTERACTION_LOADSHEDDED* codes must map to the LOADSHEDDING category to
// match legacy Rocket (RocketThriftRequests.cpp). A bug had them falling
// through to INTERNAL_ERROR, which would surface as kRocketErrorCodeCanceled
// (0x203) instead of kRocketErrorCodeRejected (0x202) on the wire — affecting
// client-side retry behavior.
TEST(ResponseErrorTest, InteractionLoadsheddedCodesMapToLoadshedding) {
  EXPECT_EQ(
      mapErrorCodeToCategory(
          apache::thrift::ResponseRpcErrorCode::INTERACTION_LOADSHEDDED),
      apache::thrift::ResponseRpcErrorCategory::LOADSHEDDING);
  EXPECT_EQ(
      mapErrorCodeToCategory(
          apache::thrift::ResponseRpcErrorCode::
              INTERACTION_LOADSHEDDED_OVERLOAD),
      apache::thrift::ResponseRpcErrorCategory::LOADSHEDDING);
  EXPECT_EQ(
      mapErrorCodeToCategory(
          apache::thrift::ResponseRpcErrorCode::
              INTERACTION_LOADSHEDDED_APP_OVERLOAD),
      apache::thrift::ResponseRpcErrorCategory::LOADSHEDDING);
  EXPECT_EQ(
      mapErrorCodeToCategory(
          apache::thrift::ResponseRpcErrorCode::
              INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT),
      apache::thrift::ResponseRpcErrorCategory::LOADSHEDDING);
}

// LOADSHEDDING and SHUTDOWN both produce REJECTED on the wire (matches
// legacy Rocket's makeRocketException). INVALID_REQUEST → INVALID,
// INTERNAL_ERROR → CANCELED.
TEST(ResponseErrorTest, MapCategoryToErrorCodeAllCategories) {
  EXPECT_EQ(
      mapCategoryToErrorCode(
          apache::thrift::ResponseRpcErrorCategory::INVALID_REQUEST),
      apache::thrift::fast_thrift::frame::ErrorCode::INVALID);
  EXPECT_EQ(
      mapCategoryToErrorCode(
          apache::thrift::ResponseRpcErrorCategory::LOADSHEDDING),
      apache::thrift::fast_thrift::frame::ErrorCode::REJECTED);
  EXPECT_EQ(
      mapCategoryToErrorCode(
          apache::thrift::ResponseRpcErrorCategory::SHUTDOWN),
      apache::thrift::fast_thrift::frame::ErrorCode::REJECTED);
  EXPECT_EQ(
      mapCategoryToErrorCode(
          apache::thrift::ResponseRpcErrorCategory::INTERNAL_ERROR),
      apache::thrift::fast_thrift::frame::ErrorCode::CANCELED);
}

} // namespace apache::thrift::fast_thrift::thrift
