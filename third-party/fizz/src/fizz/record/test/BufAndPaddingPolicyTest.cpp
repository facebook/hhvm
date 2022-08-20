/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/record/BufAndPaddingPolicy.h>

using namespace folly;

namespace fizz {
namespace test {

class BufAndPaddingPolicyTest : public testing::Test {
 protected:
  BufAndModuloPaddingPolicy modPadder_{5};
  BufAndConstPaddingPolicy constPadder_{5};

  IOBufQueue queue_{IOBufQueue::cacheChainLength()};

  static Buf
  getBuf(const std::string& hex, size_t headroom = 0, size_t tailroom = 0) {
    auto data = unhexlify(hex);
    return IOBuf::copyBuffer(data.data(), data.size(), headroom, tailroom);
  }

  void addToQueue(const std::string& hex) {
    queue_.append(getBuf(hex));
  }
};

TEST_F(BufAndPaddingPolicyTest, TestModuloQueueLessThanModulo) {
  Buf dataBuf;
  uint16_t paddingSize;
  addToQueue("1122");
  std::tie(dataBuf, paddingSize) =
      modPadder_.getBufAndPaddingToEncrypt(queue_, 50);
  EXPECT_EQ(dataBuf->computeChainDataLength(), 2);
  EXPECT_EQ(paddingSize, 3);
}

TEST_F(BufAndPaddingPolicyTest, TestModuloQueueEqualModulo) {
  Buf dataBuf;
  uint16_t paddingSize;
  addToQueue("1122334455");
  std::tie(dataBuf, paddingSize) =
      modPadder_.getBufAndPaddingToEncrypt(queue_, 50);
  EXPECT_EQ(dataBuf->computeChainDataLength(), 5);
  EXPECT_EQ(paddingSize, 0);

  addToQueue("11223344556677889900");
  std::tie(dataBuf, paddingSize) =
      modPadder_.getBufAndPaddingToEncrypt(queue_, 50);
  EXPECT_EQ(dataBuf->computeChainDataLength(), 10);
  EXPECT_EQ(paddingSize, 0);
}

TEST_F(BufAndPaddingPolicyTest, TestModuloQueueGreaterThanModulo) {
  Buf dataBuf;
  uint16_t paddingSize;
  addToQueue("112233445566");
  std::tie(dataBuf, paddingSize) =
      modPadder_.getBufAndPaddingToEncrypt(queue_, 50);
  EXPECT_EQ(dataBuf->computeChainDataLength(), 6);
  EXPECT_EQ(paddingSize, 4);

  addToQueue("112233445566778899001122");
  std::tie(dataBuf, paddingSize) =
      modPadder_.getBufAndPaddingToEncrypt(queue_, 50);
  EXPECT_EQ(dataBuf->computeChainDataLength(), 12);
  EXPECT_EQ(paddingSize, 3);
}

TEST_F(BufAndPaddingPolicyTest, TestModuloQueueLengthMax) {
  Buf dataBuf;
  uint16_t paddingSize;
  addToQueue("11223344556677889900");
  std::tie(dataBuf, paddingSize) =
      modPadder_.getBufAndPaddingToEncrypt(queue_, 10);
  EXPECT_EQ(dataBuf->computeChainDataLength(), 10);
  EXPECT_EQ(paddingSize, 0);
}

TEST_F(BufAndPaddingPolicyTest, TestModuloCloseOriginalLengthsEqualSameFinal) {
  Buf dataBuf;
  uint16_t paddingSize;
  addToQueue("112233445566");
  std::tie(dataBuf, paddingSize) =
      modPadder_.getBufAndPaddingToEncrypt(queue_, 50);
  uint16_t dataLength1 = dataBuf->computeChainDataLength();
  uint16_t paddingLength1 = paddingSize;

  addToQueue("11223344556677");
  std::tie(dataBuf, paddingSize) =
      modPadder_.getBufAndPaddingToEncrypt(queue_, 50);
  uint16_t dataLength2 = dataBuf->computeChainDataLength();
  uint16_t paddingLength2 = paddingSize;

  EXPECT_EQ(dataLength1 + paddingLength1, dataLength2 + paddingLength2);
}

TEST_F(BufAndPaddingPolicyTest, TestConstPaddingWithMoreThanPaddingSpaceLeft) {
  Buf dataBuf;
  uint16_t paddingSize;
  addToQueue("112233445566778899");
  std::tie(dataBuf, paddingSize) =
      constPadder_.getBufAndPaddingToEncrypt(queue_, 20);
  EXPECT_EQ(dataBuf->computeChainDataLength(), 9);
  EXPECT_EQ(paddingSize, 5);
}

TEST_F(BufAndPaddingPolicyTest, TestConstPaddingWithLessThanPaddingSpaceLeft) {
  Buf dataBuf;
  uint16_t paddingSize;
  addToQueue("11223344556677889900");
  std::tie(dataBuf, paddingSize) =
      constPadder_.getBufAndPaddingToEncrypt(queue_, 12);
  EXPECT_EQ(dataBuf->computeChainDataLength(), 10);
  EXPECT_EQ(paddingSize, 2);
}

TEST_F(BufAndPaddingPolicyTest, TestConstPaddingWithNoSpaceLeft) {
  Buf dataBuf;
  uint16_t paddingSize;
  addToQueue("11223344556677889900");
  std::tie(dataBuf, paddingSize) =
      constPadder_.getBufAndPaddingToEncrypt(queue_, 10);
  EXPECT_EQ(dataBuf->computeChainDataLength(), 10);
  EXPECT_EQ(paddingSize, 0);
}

} // namespace test
} // namespace fizz
