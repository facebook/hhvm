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

#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>

#include <gtest/gtest.h>

namespace apache::thrift {

class ThriftParametersServerExtensionTest : public testing::Test {
 public:
  void SetUp() override {
    context_ = std::make_shared<ThriftParametersContext>();
    extensions_ = std::make_shared<ThriftParametersServerExtension>(context_);
  }

  void setUpClientThriftParameters(
      std::vector<CompressionAlgorithm> compressionAlgos) {
    uint64_t compressionAlgorithms = 0;
    for (const auto& comp : compressionAlgos) {
      if (comp != CompressionAlgorithm::NONE) {
        compressionAlgorithms |= 1ull << (int(comp) - 1);
      }
    }
    clientThriftParams_.params.compressionAlgos() = compressionAlgorithms;
    chlo_.extensions.push_back(encodeThriftExtension(clientThriftParams_));
  }

  ThriftParametersExt clientThriftParams_;
  fizz::ClientHello chlo_;
  std::shared_ptr<ThriftParametersServerExtension> extensions_;
  std::shared_ptr<ThriftParametersContext> context_;
};

TEST_F(ThriftParametersServerExtensionTest, ServerNegotiation) {
  setUpClientThriftParameters(
      {CompressionAlgorithm::ZSTD, CompressionAlgorithm::ZLIB});
  auto exts = extensions_->getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 1);

  auto thriftParametersExtension = getThriftExtension(exts);
  EXPECT_TRUE(thriftParametersExtension.has_value());
  EXPECT_TRUE(thriftParametersExtension->params.compressionAlgos());
  EXPECT_EQ(
      *thriftParametersExtension->params.compressionAlgos(),
      1ull << (int(CompressionAlgorithm::ZSTD) - 1) |
          1ull << (int(CompressionAlgorithm::ZLIB) - 1));

  EXPECT_EQ(*thriftParametersExtension->params.pspUpgradeProtocol(), 0);
}

TEST_F(ThriftParametersServerExtensionTest, NoExtensions) {
  auto exts = extensions_->getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 0);
}

TEST_F(ThriftParametersServerExtensionTest, IncompatibleCompressionAlgorithms) {
  // The problem is that at some point CompressionAlgorithm enum stopped being a
  // continuous range. So the code below ASSUMES that 10 is the non-existing
  // value.
  // See thrift/lib/thrift/RpcMetadata.thrift.
  auto nonExistingCompressionAlgorithm = 10;

  setUpClientThriftParameters(
      {static_cast<CompressionAlgorithm>(nonExistingCompressionAlgorithm)});

  auto exts = extensions_->getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 1);

  auto thriftParametersExtension = getThriftExtension(exts);
  EXPECT_TRUE(thriftParametersExtension.has_value());
  EXPECT_TRUE(thriftParametersExtension->params.compressionAlgos());
  EXPECT_EQ(
      *thriftParametersExtension->params.compressionAlgos(),
      1ull << (int(CompressionAlgorithm::ZSTD) - 1) |
          1ull << (int(CompressionAlgorithm::ZLIB) - 1));
}

class ThriftServerPSPNegotiationTest : public testing::Test {
 public:
  struct NegotiationResult {
    std::shared_ptr<ThriftParametersServerExtension> ext;
    NegotiationParameters sentParameters;
  };

  NegotiationResult negotiate(
      folly::Function<ThriftParametersContext::PSPNegotiationPolicy>
          serverPolicy,
      const folly::SocketAddress& peerAddr,
      folly::Optional<uint64_t> clientAdvertisedPSP) {
    ThriftParametersExt clientExt;
    if (clientAdvertisedPSP.has_value()) {
      clientExt.params.pspUpgradeProtocol() = *clientAdvertisedPSP;
    }
    auto ctx = std::make_shared<ThriftParametersContext>();
    if (serverPolicy) {
      ctx->setSupportedPSPVersionsPolicy(std::move(serverPolicy));
    }
    ctx->setPeerAddress(peerAddr);

    auto serverExt = std::make_shared<ThriftParametersServerExtension>(ctx);

    fizz::ClientHello chlo;
    chlo.extensions.push_back(encodeThriftExtension(clientExt));
    auto sentExtensions = serverExt->getExtensions(chlo);

    EXPECT_EQ(sentExtensions.size(), 1);

    auto thriftParametersExtension = getThriftExtension(sentExtensions);
    EXPECT_TRUE(thriftParametersExtension.has_value());

    return NegotiationResult{
        std::move(serverExt), thriftParametersExtension.value().params};
  }
};

// The default policy is to reject any client psp upgrade attempts
TEST_F(ThriftServerPSPNegotiationTest, NoPolicySet) {
  auto r = negotiate({}, {}, ~0ULL);
  EXPECT_EQ((unsigned long)(*r.sentParameters.pspUpgradeProtocol()), 0L);
  EXPECT_EQ(r.ext->getNegotiatedPSPUpgrade(), 0L);
}

// Server supported versions are disjoint from client advertised
TEST_F(ThriftServerPSPNegotiationTest, ServerClientDisjoint) {
  auto r = negotiate([](const auto&) { return 0x8; }, {}, 0x1);
  EXPECT_EQ((unsigned long)(*r.sentParameters.pspUpgradeProtocol()), 0L);
  EXPECT_EQ(r.ext->getNegotiatedPSPUpgrade(), 0L);
}

// Server and client have overlapping versions. Server chooses highest
TEST_F(ThriftServerPSPNegotiationTest, ServerAcceptsClientHighest) {
  auto r = negotiate([](const auto&) { return 0xf; }, {}, 0x7);
  EXPECT_EQ((unsigned long)(*r.sentParameters.pspUpgradeProtocol()), 4L);
  EXPECT_EQ(r.ext->getNegotiatedPSPUpgrade(), 4L);
}

// A negotiation policy which changes which each invocation is properly handled
TEST_F(ThriftServerPSPNegotiationTest, ServerNegotiatesConsistently) {
  size_t count = 0;
  auto r = negotiate([&](const auto&) { return (1 << (count++)); }, {}, 0xf);
  EXPECT_EQ((unsigned long)(*r.sentParameters.pspUpgradeProtocol()), 1L);

  // This test verifies that getNegotiatedPSPUpgrade is not attempting to
  // recompute "what it would have sent" during each invocation, since
  // computing what it would have sent would require a policy function
  // invocation
  EXPECT_EQ(r.ext->getNegotiatedPSPUpgrade(), 1L);
}

} // namespace apache::thrift
