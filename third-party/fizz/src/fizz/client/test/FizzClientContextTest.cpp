/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/client/ClientProtocol.h>
#include <fizz/client/FizzClientContext.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/protocol/test/ProtocolTest.h>

using namespace fizz::test;

namespace fizz {
namespace client {
namespace test {

class FizzClientContextTest : public ::testing::Test {
 public:
  void SetUp() override {
    auto mockFactory = std::make_shared<MockFactory>();
    mockFactory->setDefaults();
    factory_ = mockFactory.get();

    context_ = std::make_shared<FizzClientContext>(mockFactory);
  }

  void expectValidateThrows(std::string msg) {
    try {
      context_->validate();
    } catch (const std::exception& error) {
      EXPECT_THAT(error.what(), HasSubstr(msg));
      return;
    }
    // shouldn't reach here
    ASSERT_TRUE(false);
  }

  std::shared_ptr<FizzClientContext> context_;
  MockFactory* factory_;
};

TEST_F(FizzClientContextTest, TestValidateUnsupportedCipher) {
  const auto unsupportedCipher = static_cast<fizz::CipherSuite>(0xFFFF);
  EXPECT_CALL(*factory_, makeAead(_)).WillRepeatedly([](CipherSuite cipher) {
    if (cipher == unsupportedCipher) {
      throw std::runtime_error("unsupported cipher");
    } else {
      return std::make_unique<MockAead>();
    }
  });

  context_->setSupportedCiphers({unsupportedCipher});

  expectValidateThrows("unsupported cipher");
}

TEST_F(FizzClientContextTest, TestValidateUnsupportedGroup) {
  const auto unsupportedGroup = static_cast<fizz::NamedGroup>(0xFFFF);
  EXPECT_CALL(*factory_, makeKeyExchange(_, _))
      .WillRepeatedly([](NamedGroup group, KeyExchangeRole /*unused*/) {
        if (group == unsupportedGroup) {
          throw std::runtime_error("unsupported group");
        } else {
          return std::make_unique<MockKeyExchange>();
        }
      });

  context_->setSupportedGroups({unsupportedGroup});

  expectValidateThrows("unsupported group");
}

TEST_F(FizzClientContextTest, TestValidateUnsupportedDefaultShare) {
  context_->setSupportedGroups(
      {static_cast<fizz::NamedGroup>(0x01),
       static_cast<fizz::NamedGroup>(0x02)});

  context_->setDefaultShares(
      {static_cast<fizz::NamedGroup>(0x02),
       static_cast<fizz::NamedGroup>(0x03)});

  expectValidateThrows("unsupported named group in default shares");
}

TEST_F(FizzClientContextTest, TestValidateSuccess) {
  EXPECT_CALL(*factory_, makeAead(_)).WillRepeatedly([](CipherSuite cipher) {
    if (cipher == static_cast<fizz::CipherSuite>(0xFFFF)) {
      throw std::runtime_error("unsupported cipher");
    } else {
      return std::make_unique<MockAead>();
    }
  });
  EXPECT_CALL(*factory_, makeKeyExchange(_, _))
      .WillRepeatedly([](NamedGroup group, KeyExchangeRole /*unused*/) {
        if (group == static_cast<fizz::NamedGroup>(0xFFFF)) {
          throw std::runtime_error("unsupported group");
        } else {
          return std::make_unique<MockKeyExchange>();
        }
      });

  context_->setSupportedCiphers(
      {static_cast<fizz::CipherSuite>(0x01),
       static_cast<fizz::CipherSuite>(0x02)});

  context_->setSupportedGroups(
      {static_cast<fizz::NamedGroup>(0x03),
       static_cast<fizz::NamedGroup>(0x04)});

  context_->setDefaultShares({static_cast<fizz::NamedGroup>(0x03)});

  EXPECT_NO_THROW(context_->validate());
}
} // namespace test
} // namespace client
} // namespace fizz
