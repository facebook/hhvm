/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/server/DualTicketCipher.h>
#include <fizz/server/test/Mocks.h>

namespace fizz {
namespace server {
namespace test {

TEST(DualCipherTest, Encryption) {
  auto cipher = std::make_unique<MockTicketCipher>();
  auto fallbackCipher = std::make_unique<MockTicketCipher>();
  ResumptionState resState;

  cipher->setDefaults();
  fallbackCipher->setDefaults();
  EXPECT_CALL(*cipher, _encrypt(_)).Times(1);
  EXPECT_CALL(*fallbackCipher, _encrypt(_)).Times(0);

  auto dualCipher =
      DualTicketCipher(std::move(cipher), std::move(fallbackCipher));
  dualCipher.encrypt(std::move(resState));
}

TEST(DualCipherTest, DecryptSuccess) {
  auto cipher = std::make_unique<MockTicketCipher>();
  auto fallbackCipher = std::make_unique<MockTicketCipher>();
  ResumptionState resState;

  EXPECT_CALL(*cipher, _decrypt(_)).WillOnce(InvokeWithoutArgs([]() {
    ResumptionState res;
    return std::make_pair(PskType::Resumption, std::move(res));
  }));
  EXPECT_CALL(*fallbackCipher, _decrypt(_)).Times(0);

  auto dualCipher =
      DualTicketCipher(std::move(cipher), std::move(fallbackCipher));
  auto buf =
      folly::IOBuf::wrapBuffer(folly::ByteRange(folly::StringPiece("ss")));
  dualCipher.decrypt(std::move(buf));
}

TEST(DualCipherTest, DecryptSuccessWithFallback) {
  auto cipher = std::make_unique<MockTicketCipher>();
  auto fallbackCipher = std::make_unique<MockTicketCipher>();
  ResumptionState resState;

  EXPECT_CALL(*cipher, _decrypt(_)).WillOnce(InvokeWithoutArgs([]() {
    ResumptionState res;
    return std::make_pair(PskType::Rejected, std::move(res));
  }));
  EXPECT_CALL(*fallbackCipher, _decrypt(_)).WillOnce(InvokeWithoutArgs([]() {
    ResumptionState res;
    return std::make_pair(PskType::Resumption, std::move(res));
  }));

  auto dualCipher =
      DualTicketCipher(std::move(cipher), std::move(fallbackCipher));
  auto buf =
      folly::IOBuf::wrapBuffer(folly::ByteRange(folly::StringPiece("ss")));
  EXPECT_EQ(
      std::get<0>(dualCipher.decrypt(std::move(buf)).get()),
      PskType::Resumption);
}
} // namespace test
} // namespace server
} // namespace fizz
