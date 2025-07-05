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

} // namespace apache::thrift
