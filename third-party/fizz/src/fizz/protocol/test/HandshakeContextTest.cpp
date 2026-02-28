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
  auto context = fizz::DefaultFactory().makeHandshakeContext(
      CipherSuite::TLS_AES_128_GCM_SHA256);
  context->appendToTranscript(folly::IOBuf::copyBuffer("ClientHello"));
  auto c = context->getHandshakeContext();
  EXPECT_EQ(
      folly::hexlify(c->coalesce()),
      "b001745d730d53a71b509ee6ed8a09d57c5cd2ebad255f8aafda37d88dc2d836");
}

TEST_F(HandshakeContextTest, TestHandshakeContextMultiple) {
  auto context = fizz::DefaultFactory().makeHandshakeContext(
      CipherSuite::TLS_AES_128_GCM_SHA256);
  context->appendToTranscript(folly::IOBuf::copyBuffer("ClientHello"));
  context->appendToTranscript(folly::IOBuf::copyBuffer("ServerHello"));
  auto c = context->getHandshakeContext();
  EXPECT_EQ(
      folly::hexlify(c->coalesce()),
      "1314fc0610c6d9b5ef3668f71239998a8a65a21ad377490bc391888ac80c56a7");
}

TEST_F(HandshakeContextTest, TestFinished) {
  auto context = fizz::DefaultFactory().makeHandshakeContext(
      CipherSuite::TLS_AES_128_GCM_SHA256);
  context->appendToTranscript(folly::IOBuf::copyBuffer("ClientHello"));
  std::vector<uint8_t> baseKey(Sha256::HashLen);
  auto f = context->getFinishedData(range(baseKey));
  EXPECT_EQ(
      folly::hexlify(f->coalesce()),
      "296d7f5fea7788fc33b3596e55df776b3bd63f874db5742a8cba718741411f9d");
}

TEST_F(HandshakeContextTest, TestEmpty) {
  auto context = fizz::DefaultFactory().makeHandshakeContext(
      CipherSuite::TLS_AES_128_GCM_SHA256);
  context->getHandshakeContext();
  std::array<uint8_t, Sha256::HashLen> key{4};
  auto f = context->getFinishedData(folly::range(key));
  EXPECT_EQ(
      folly::hexlify(f->coalesce()),
      "d3bd065ddc9f600cc3674e0f9f2f14e3f8d11185d92768606c7597f145c3d6cf");
}
} // namespace test
} // namespace fizz
