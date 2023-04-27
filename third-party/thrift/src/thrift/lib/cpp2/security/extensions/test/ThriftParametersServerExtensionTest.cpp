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

#include <folly/portability/GTest.h>

namespace apache {
namespace thrift {

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
    clientThriftParams_.params.compressionAlgos_ref() = compressionAlgorithms;
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
  EXPECT_TRUE(thriftParametersExtension->params.compressionAlgos_ref());
  EXPECT_EQ(
      *thriftParametersExtension->params.compressionAlgos_ref(),
      1ull << (int(CompressionAlgorithm::ZSTD) - 1) |
          1ull << (int(CompressionAlgorithm::ZLIB) - 1));
}

TEST_F(ThriftParametersServerExtensionTest, NoExtensions) {
  auto exts = extensions_->getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 0);
}

TEST_F(ThriftParametersServerExtensionTest, IncompatibleCompresionAlgorithms) {
  setUpClientThriftParameters({
      static_cast<CompressionAlgorithm>(
          folly::to_underlying(TEnumTraits<CompressionAlgorithm>::max()) +
          1u), // definitely fake
  });

  auto exts = extensions_->getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 1);

  auto thriftParametersExtension = getThriftExtension(exts);
  EXPECT_TRUE(thriftParametersExtension.has_value());
  EXPECT_TRUE(thriftParametersExtension->params.compressionAlgos_ref());
  EXPECT_EQ(
      *thriftParametersExtension->params.compressionAlgos_ref(),
      1ull << (int(CompressionAlgorithm::ZSTD) - 1) |
          1ull << (int(CompressionAlgorithm::ZLIB) - 1));
}

} // namespace thrift
} // namespace apache
