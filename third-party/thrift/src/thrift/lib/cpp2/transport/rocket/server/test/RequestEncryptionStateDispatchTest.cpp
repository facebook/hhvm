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
#include <thrift/lib/cpp2/security/SSLUtil.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/RequestEncryptionState.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RequestEncryptionStateDispatch.h>

using namespace apache::thrift;
using namespace apache::thrift::rocket::context_utils;

THRIFT_FLAG_DECLARE_bool(server_request_encryption_tracking_enabled);

// --- flag-off tests ---

TEST(RequestEncryptionStateDispatchTest, FlagOffNoOp) {
  THRIFT_FLAG_SET_MOCK(server_request_encryption_tracking_enabled, false);

  Cpp2ConnContext connCtx;
  detail::Cpp2ConnContextInternalAPI(connCtx).setSecurityProtocol("TLS");
  Cpp2RequestContext reqCtx(&connCtx);

  checkRequestEncryptionState(reqCtx);

  // flag off, so state stays at default Plaintext (unchanged)
  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Plaintext);
}

// --- null connection context tests ---

TEST(RequestEncryptionStateDispatchTest, NullConnectionContext) {
  THRIFT_FLAG_SET_MOCK(server_request_encryption_tracking_enabled, true);

  // construct Cpp2RequestContext with null connCtx
  Cpp2RequestContext reqCtx(nullptr);

  checkRequestEncryptionState(reqCtx);

  // null connCtx → no crash, state stays Plaintext
  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Plaintext);
}

// --- plaintext connection tests ---

TEST(RequestEncryptionStateDispatchTest, PlaintextConnection) {
  THRIFT_FLAG_SET_MOCK(server_request_encryption_tracking_enabled, true);

  Cpp2ConnContext connCtx;
  // default Cpp2ConnContext has empty securityProtocol (plaintext)
  Cpp2RequestContext reqCtx(&connCtx);

  checkRequestEncryptionState(reqCtx);

  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Plaintext);
}

// --- encrypted connection tests ---

TEST(RequestEncryptionStateDispatchTest, TlsConnection) {
  THRIFT_FLAG_SET_MOCK(server_request_encryption_tracking_enabled, true);

  Cpp2ConnContext connCtx;
  detail::Cpp2ConnContextInternalAPI(connCtx).setSecurityProtocol("TLS");
  Cpp2RequestContext reqCtx(&connCtx);

  checkRequestEncryptionState(reqCtx);

  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Encrypted);
}

TEST(RequestEncryptionStateDispatchTest, FizzConnection) {
  THRIFT_FLAG_SET_MOCK(server_request_encryption_tracking_enabled, true);

  Cpp2ConnContext connCtx;
  detail::Cpp2ConnContextInternalAPI(connCtx).setSecurityProtocol("Fizz");
  Cpp2RequestContext reqCtx(&connCtx);

  checkRequestEncryptionState(reqCtx);

  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Encrypted);
}

TEST(RequestEncryptionStateDispatchTest, PspConnection) {
  THRIFT_FLAG_SET_MOCK(server_request_encryption_tracking_enabled, true);

  Cpp2ConnContext connCtx;
  detail::Cpp2ConnContextInternalAPI(connCtx).setSecurityProtocol(
      "thriftPSPV0");
  Cpp2RequestContext reqCtx(&connCtx);

  checkRequestEncryptionState(reqCtx);

  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Encrypted);
}

TEST(RequestEncryptionStateDispatchTest, FizzKtlsConnection) {
  THRIFT_FLAG_SET_MOCK(server_request_encryption_tracking_enabled, true);

  Cpp2ConnContext connCtx;
  // Fizz/KTLS is the kernel-TLS offload path — encrypted on the wire,
  // counted as Encrypted by isTransportEncrypted().
  detail::Cpp2ConnContextInternalAPI(connCtx).setSecurityProtocol("Fizz/KTLS");
  Cpp2RequestContext reqCtx(&connCtx);

  checkRequestEncryptionState(reqCtx);

  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Encrypted);
}

TEST(RequestEncryptionStateDispatchTest, StopTlsConnection_Plaintext) {
  THRIFT_FLAG_SET_MOCK(server_request_encryption_tracking_enabled, true);

  Cpp2ConnContext connCtx;
  // "stopTLS" (the older variant, pre-v2) is not encrypted per
  // isTransportEncrypted()
  detail::Cpp2ConnContextInternalAPI(connCtx).setSecurityProtocol("stopTLS");
  Cpp2RequestContext reqCtx(&connCtx);

  checkRequestEncryptionState(reqCtx);

  // not StopTLSv2 → falls through to per-connection logic, stopTLS → not
  // encrypted
  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(), RequestEncryptionState::Plaintext);
}

// --- stoptlsv2 fail-safe test ---

TEST(RequestEncryptionStateDispatchTest, StopTLSv2Connection_NoTransport) {
  THRIFT_FLAG_SET_MOCK(server_request_encryption_tracking_enabled, true);

  Cpp2ConnContext connCtx;
  detail::Cpp2ConnContextInternalAPI(connCtx).setSecurityProtocol(
      kSecurityProtocolStopTLSV2);
  // no transport set → record layer unreachable
  Cpp2RequestContext reqCtx(&connCtx);

  checkRequestEncryptionState(reqCtx);

  // StopTLSv2 but no transport → fail safe to StoptlsSkipped (can't prove
  // encryption)
  EXPECT_EQ(
      reqCtx.getRequestEncryptionState(),
      RequestEncryptionState::StoptlsSkipped);
}

// TODO(fiqi): add positive-case integration test that validates
// checkRequestEncryptionState() actually sets StoptlsEncrypted vs
// StoptlsSkipped when a StopTLSv2 connection has/hasn't observed plaintext.
// Requires heavyweight fizz fixture setup (real AsyncFizzServer +
// CompositeReadRecordLayer with StopTLSv2 negotiated + real traffic). Manual
// canary validation covers this in the meantime via Scuba query for
// REQUEST_ENCRYPTION_STATE={StoptlsEncrypted,StoptlsSkipped} on known
// connections.
