/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/compression/CertDecompressionManager.h>
#include <fizz/compression/test/Mocks.h>

namespace fizz {
namespace test {

class CertDecompressionManagerTest : public testing::Test {
 public:
  void SetUp() override {
    manager_ = std::make_unique<CertDecompressionManager>();
  }

  void TearDown() override {}

 protected:
  CertificateCompressionAlgorithm toAlgo(uint16_t n) {
    return static_cast<CertificateCompressionAlgorithm>(n);
  }

  std::shared_ptr<MockCertificateDecompressor> makeMockDecompressor(
      uint16_t n) {
    auto comp = std::make_shared<MockCertificateDecompressor>();
    EXPECT_CALL(*comp, getAlgorithm()).WillOnce(Return(toAlgo(n)));
    return comp;
  }

  std::unique_ptr<CertDecompressionManager> manager_;
};

TEST_F(CertDecompressionManagerTest, TestBasic) {
  auto decomp1 = makeMockDecompressor(1);
  auto decomp2 = makeMockDecompressor(2);

  manager_->setDecompressors(
      {std::static_pointer_cast<CertificateDecompressor>(decomp1),
       std::static_pointer_cast<CertificateDecompressor>(decomp2)});

  auto supported = manager_->getSupportedAlgorithms();
  EXPECT_EQ(supported.size(), 2);
  EXPECT_EQ(supported[0], toAlgo(1));
  EXPECT_EQ(supported[1], toAlgo(2));
  auto fetchedDecomp1 = manager_->getDecompressor(toAlgo(1));
  auto fetchedDecomp2 = manager_->getDecompressor(toAlgo(2));
  auto invalidDecomp = manager_->getDecompressor(toAlgo(3));
  EXPECT_EQ(decomp1, fetchedDecomp1);
  EXPECT_EQ(decomp2, fetchedDecomp2);
  EXPECT_EQ(invalidDecomp, nullptr);
}

TEST_F(CertDecompressionManagerTest, TestSameOverwrites) {
  auto decomp1 = makeMockDecompressor(1);
  auto decomp2 = makeMockDecompressor(1);
  auto decomp3 = makeMockDecompressor(2);

  manager_->setDecompressors(
      {std::static_pointer_cast<CertificateDecompressor>(decomp1),
       std::static_pointer_cast<CertificateDecompressor>(decomp2),
       std::static_pointer_cast<CertificateDecompressor>(decomp3)});

  auto supported = manager_->getSupportedAlgorithms();
  EXPECT_EQ(supported.size(), 2);
  EXPECT_EQ(supported[0], toAlgo(1));
  EXPECT_EQ(supported[1], toAlgo(2));
  auto fetchedDecomp1 = manager_->getDecompressor(toAlgo(1));
  auto fetchedDecomp2 = manager_->getDecompressor(toAlgo(2));
  EXPECT_EQ(decomp2, fetchedDecomp1);
  EXPECT_NE(decomp1, fetchedDecomp1);
  EXPECT_EQ(decomp3, fetchedDecomp2);
}

} // namespace test
} // namespace fizz
