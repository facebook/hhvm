/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/protocol/HandshakeContext.h>

using namespace folly;

namespace fizz {
namespace test {

// TODO: test vectors
class HandshakeContextTest : public testing::Test {};

TEST_F(HandshakeContextTest, TestHandshakeContextSingle) {
  HandshakeContextImpl<openssl::Sha256> context(kHkdfLabelPrefix.str());
  context.appendToTranscript(folly::IOBuf::copyBuffer("ClientHello"));
  context.getHandshakeContext();
}

TEST_F(HandshakeContextTest, TestHandshakeContextMultiple) {
  HandshakeContextImpl<openssl::Sha256> context(kHkdfLabelPrefix.str());
  context.appendToTranscript(folly::IOBuf::copyBuffer("ClientHello"));
  context.appendToTranscript(folly::IOBuf::copyBuffer("ServerHello"));
  context.getHandshakeContext();
}

TEST_F(HandshakeContextTest, TestFinished) {
  HandshakeContextImpl<openssl::Sha256> context(kHkdfLabelPrefix.str());
  context.appendToTranscript(folly::IOBuf::copyBuffer("ClientHello"));
  std::vector<uint8_t> baseKey(openssl::Sha256::HashLen);
  context.getFinishedData(range(baseKey));
}

TEST_F(HandshakeContextTest, TestEmpty) {
  HandshakeContextImpl<openssl::Sha256> context(kHkdfLabelPrefix.str());
  context.getHandshakeContext();
  std::array<uint8_t, openssl::Sha256::HashLen> key{4};
  context.getFinishedData(folly::range(key));
}
} // namespace test
} // namespace fizz
