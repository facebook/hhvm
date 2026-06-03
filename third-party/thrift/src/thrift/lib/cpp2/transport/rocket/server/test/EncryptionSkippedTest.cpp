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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/RequestEncryptionState.h>
// RocketContextUtils.h is included to pull in the THRIFT_FLAG_DEFINE for
// server_request_encryption_tracking_enabled (defined in
// RequestEncryptionStateDispatch.cpp, linked via
// :RequestEncryptionStateDispatch).
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketContextUtils.h>

THRIFT_FLAG_DECLARE_bool(server_request_encryption_tracking_enabled);

using namespace apache::thrift;

// --- Cpp2RequestContext tests ---

TEST(EncryptionSkippedTest, RequestContextDefaultsToPlaintext) {
  Cpp2ConnContext connCtx;
  Cpp2RequestContext reqCtx(&connCtx);
  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Plaintext);
}

TEST(EncryptionSkippedTest, RequestContextSetEncrypted) {
  Cpp2ConnContext connCtx;
  Cpp2RequestContext reqCtx(&connCtx);
  reqCtx.setRequestEncryptionState(RequestEncryptionState::Encrypted);
  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Encrypted);
}

TEST(EncryptionSkippedTest, RequestContextSetStateMultipleTransitions) {
  Cpp2ConnContext connCtx;
  Cpp2RequestContext reqCtx(&connCtx);
  reqCtx.setRequestEncryptionState(RequestEncryptionState::Encrypted);
  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Encrypted);
  reqCtx.setRequestEncryptionState(RequestEncryptionState::StoptlsEncrypted);
  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(),
      RequestEncryptionState::StoptlsEncrypted);
  reqCtx.setRequestEncryptionState(RequestEncryptionState::StoptlsSkipped);
  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(),
      RequestEncryptionState::StoptlsSkipped);
  reqCtx.setRequestEncryptionState(RequestEncryptionState::Plaintext);
  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Plaintext);
}

// --- THRIFT_FLAG tests ---

TEST(EncryptionSkippedTest, ThriftFlagDefaultsToFalse) {
  EXPECT_FALSE(THRIFT_FLAG(server_request_encryption_tracking_enabled));
}

TEST(EncryptionSkippedTest, ThriftFlagMockEnable) {
  THRIFT_FLAG_SET_MOCK(server_request_encryption_tracking_enabled, true);
  EXPECT_TRUE(THRIFT_FLAG(server_request_encryption_tracking_enabled));
}

// --- Multiple requests on same connection ---

TEST(EncryptionSkippedTest, MultipleRequestsIndependent) {
  Cpp2ConnContext connCtx;
  Cpp2RequestContext reqCtx1(&connCtx);
  Cpp2RequestContext reqCtx2(&connCtx);

  reqCtx1.setRequestEncryptionState(RequestEncryptionState::StoptlsSkipped);
  EXPECT_EQ(
      reqCtx1.getRequestEncryptionState(),
      RequestEncryptionState::StoptlsSkipped);
  // reqCtx2 is independent and should still be Plaintext
  EXPECT_EQ(
      reqCtx2.getRequestEncryptionState(), RequestEncryptionState::Plaintext);
}
