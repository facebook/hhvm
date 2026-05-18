/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/protocol/DefaultFactory.h>
#include <fizz/protocol/HandshakeContext.h>

using namespace folly;

namespace fizz {
namespace test {

// TODO: test vectors
class HandshakeContextTest : public testing::Test {};

TEST_F(HandshakeContextTest, TestHandshakeContextSingle) {
  std::unique_ptr<HandshakeContext> context;
  Error err;
  EXPECT_EQ(
      fizz::DefaultFactory().makeHandshakeContext(
          context, err, CipherSuite::TLS_AES_128_GCM_SHA256),
      Status::Success);
  EXPECT_EQ(
      context->appendToTranscript(err, folly::IOBuf::copyBuffer("ClientHello")),
      Status::Success);
  Buf c;
  EXPECT_EQ(context->getHandshakeContext(c, err), Status::Success);
  EXPECT_EQ(
      folly::hexlify(c->coalesce()),
      "b001745d730d53a71b509ee6ed8a09d57c5cd2ebad255f8aafda37d88dc2d836");
}

TEST_F(HandshakeContextTest, TestHandshakeContextMultiple) {
  std::unique_ptr<HandshakeContext> context;
  Error err;
  EXPECT_EQ(
      fizz::DefaultFactory().makeHandshakeContext(
          context, err, CipherSuite::TLS_AES_128_GCM_SHA256),
      Status::Success);
  EXPECT_EQ(
      context->appendToTranscript(err, folly::IOBuf::copyBuffer("ClientHello")),
      Status::Success);
  EXPECT_EQ(
      context->appendToTranscript(err, folly::IOBuf::copyBuffer("ServerHello")),
      Status::Success);
  Buf c;
  EXPECT_EQ(context->getHandshakeContext(c, err), Status::Success);
  EXPECT_EQ(
      folly::hexlify(c->coalesce()),
      "1314fc0610c6d9b5ef3668f71239998a8a65a21ad377490bc391888ac80c56a7");
}

TEST_F(HandshakeContextTest, TestFinished) {
  std::unique_ptr<HandshakeContext> context;
  Error err;
  EXPECT_EQ(
      fizz::DefaultFactory().makeHandshakeContext(
          context, err, CipherSuite::TLS_AES_128_GCM_SHA256),
      Status::Success);
  EXPECT_EQ(
      context->appendToTranscript(err, folly::IOBuf::copyBuffer("ClientHello")),
      Status::Success);
  std::vector<uint8_t> baseKey(Sha256::HashLen);
  Buf f;
  EXPECT_EQ(context->getFinishedData(f, err, range(baseKey)), Status::Success);
  EXPECT_EQ(
      folly::hexlify(f->coalesce()),
      "296d7f5fea7788fc33b3596e55df776b3bd63f874db5742a8cba718741411f9d");
}

TEST_F(HandshakeContextTest, TestEmpty) {
  std::unique_ptr<HandshakeContext> context;
  Error err;
  EXPECT_EQ(
      fizz::DefaultFactory().makeHandshakeContext(
          context, err, CipherSuite::TLS_AES_128_GCM_SHA256),
      Status::Success);
  Buf hashBuf;
  EXPECT_EQ(context->getHandshakeContext(hashBuf, err), Status::Success);
  std::array<uint8_t, Sha256::HashLen> key{4};
  Buf f;
  EXPECT_EQ(
      context->getFinishedData(f, err, folly::range(key)), Status::Success);
  EXPECT_EQ(
      folly::hexlify(f->coalesce()),
      "d3bd065ddc9f600cc3674e0f9f2f14e3f8d11185d92768606c7597f145c3d6cf");
}
} // namespace test
} // namespace fizz
