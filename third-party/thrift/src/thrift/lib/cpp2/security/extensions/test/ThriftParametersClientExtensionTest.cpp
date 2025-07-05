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

#include <thrift/lib/cpp2/security/extensions/ThriftParametersClientExtension.h>

#include <gtest/gtest.h>

namespace apache::thrift {

class ThriftParametersClientExtensionTest : public testing::Test {
 public:
  void SetUp() override {
    context_ = std::make_shared<ThriftParametersContext>();
    extensions_ = std::make_shared<ThriftParametersClientExtension>(context_);
  }

  void setUpServerHelloExtensions(
      std::vector<CompressionAlgorithm> compressionAlgos) {
    NegotiationParameters params;
    uint64_t compressionAlgorithms = 0;
    for (const auto& comp : compressionAlgos) {
      if (comp != CompressionAlgorithm::NONE) {
        LOG(INFO) << "Compression: " << (int)comp;
        compressionAlgorithms |= 1ull << (int(comp) - 1);
      }
    }
    params.compressionAlgos() = compressionAlgorithms;
    ThriftParametersExt paramsExt;
    paramsExt.params = params;
    serverExtensions_.push_back(encodeThriftExtension(paramsExt));
  }

  std::vector<fizz::Extension> serverExtensions_;
  std::shared_ptr<ThriftParametersClientExtension> extensions_;
  std::shared_ptr<ThriftParametersContext> context_;
};

TEST_F(ThriftParametersClientExtensionTest, ValidExtensions) {
  setUpServerHelloExtensions({CompressionAlgorithm::ZSTD});
  extensions_->onEncryptedExtensions(serverExtensions_);
  EXPECT_TRUE(extensions_->getThriftCompressionAlgorithm().has_value());
  EXPECT_EQ(
      extensions_->getThriftCompressionAlgorithm(), CompressionAlgorithm::ZSTD);
}

TEST_F(ThriftParametersClientExtensionTest, NoExtensions) {
  extensions_->onEncryptedExtensions(serverExtensions_);
  EXPECT_FALSE(extensions_->getThriftCompressionAlgorithm().has_value());
}

TEST_F(ThriftParametersClientExtensionTest, ServerMismatchCompressionAlgo) {
  // The problem is that at some point CompressionAlgorithm enum stopped being a
  // continuous range. So the code below ASSUMES that 10 is the non-existing
  // value.
  // See thrift/lib/thrift/RpcMetadata.thrift.
  auto nonExistingCompressionAlgorithm = 10;
  setUpServerHelloExtensions(
      {static_cast<CompressionAlgorithm>(nonExistingCompressionAlgorithm)});
  extensions_->onEncryptedExtensions(serverExtensions_);
  EXPECT_FALSE(extensions_->getThriftCompressionAlgorithm().has_value());
}

TEST_F(ThriftParametersClientExtensionTest, ServerMultipleCompressionAlgo) {
  setUpServerHelloExtensions(
      {CompressionAlgorithm::ZSTD, CompressionAlgorithm::ZLIB});
  extensions_->onEncryptedExtensions(serverExtensions_);

  EXPECT_TRUE(extensions_->getThriftCompressionAlgorithm().has_value());
  EXPECT_EQ(
      extensions_->getThriftCompressionAlgorithm(), CompressionAlgorithm::ZSTD);
}
} // namespace apache::thrift
