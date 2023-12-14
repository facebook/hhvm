/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/clock/test/Mocks.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/server/TicketTypes.h>
#include <fizz/util/FizzUtil.h>
#include <folly/FileUtil.h>
#include <folly/experimental/TestUtil.h>

namespace fizz {
namespace test {

TEST(UtilTest, GetAlpnFromNpn) {
  std::list<folly::SSLContext::NextProtocolsItem> npList;
  std::list<std::string> protocolList1{"test", "test2"};
  std::list<std::string> protocolList2{"test3", "test4"};

  npList.push_back(folly::SSLContext::NextProtocolsItem(1, protocolList1));
  {
    std::vector<std::string> expectedList{
        std::begin(protocolList1), std::end(protocolList1)};
    EXPECT_EQ(FizzUtil::getAlpnsFromNpnList(npList), expectedList);
  }

  npList.push_back(folly::SSLContext::NextProtocolsItem(2, protocolList2));
  {
    std::vector<std::string> expectedList{
        std::begin(protocolList2), std::end(protocolList2)};
    EXPECT_EQ(FizzUtil::getAlpnsFromNpnList(npList), expectedList);
  }
}

TEST(UtilTest, CreateTicketCipher) {
  auto cipher = FizzUtil::createTicketCipher<server::AES128TicketCipher>(
      std::vector<std::string>(),
      "fakeSecretttttttttttttttttttttttttt",
      std::vector<std::string>(),
      std::chrono::seconds(100),
      std::chrono::minutes(100),
      std::make_shared<MockFactory>(),
      std::make_shared<server::CertManager>(),
      folly::Optional<std::string>("fakeContext"));
  auto clock = std::make_shared<MockClock>();
  server::TicketPolicy policy;
  policy.setClock(clock);
  cipher->setPolicy(policy);
  {
    server::ResumptionState state;
    auto blob = cipher->encrypt(std::move(state)).get();
    EXPECT_EQ(
        std::get<0>(cipher->decrypt(std::move(std::get<0>(*blob))).get()),
        PskType::Resumption);
  }
  {
    auto newCipher = FizzUtil::createTicketCipher<server::AES128TicketCipher>(
        std::vector<std::string>(),
        "fakeSecrettttttttttttttttttttttttt2",
        std::vector<std::string>(),
        std::chrono::seconds(100),
        std::chrono::minutes(100),
        std::make_shared<MockFactory>(),
        std::make_shared<server::CertManager>(),
        folly::Optional<std::string>("fakeContext"));
    newCipher->setPolicy(std::move(policy));
    server::ResumptionState state;
    auto blob = cipher->encrypt(std::move(state)).get();
    EXPECT_EQ(
        std::get<0>(newCipher->decrypt(std::move(std::get<0>(*blob))).get()),
        PskType::Rejected);
  }
}

TEST(UtilTest, CreateTokenCipher) {
  auto cipher = FizzUtil::createTokenCipher<server::AES128TokenCipher>(
      std::vector<std::string>(),
      "fakeSecrettttttttttttttttttttttttt3",
      std::vector<std::string>(),
      folly::Optional<std::string>("fakePskContext"),
      std::string("fakeCodecContext"));
  {
    auto inMessage = "Secret message";
    auto inPlaintextBuf = folly::IOBuf::copyBuffer(inMessage);
    auto cipherTextBuf = cipher->encrypt(std::move(inPlaintextBuf)).value();
    auto outPlaintextBuf = cipher->decrypt(std::move(cipherTextBuf)).value();
    auto outMessage = outPlaintextBuf->moveToFbString().toStdString();
    EXPECT_EQ(inMessage, outMessage);
  }
  {
    auto newCipher = FizzUtil::createTokenCipher<server::AES128TokenCipher>(
        std::vector<std::string>(),
        "fakeSecrettttttttttttttttttttttttt4",
        std::vector<std::string>(),
        folly::Optional<std::string>("fakePskContext"),
        std::string("fakeCodecContext"));
    auto inMessage = "Brand new secret message";
    auto inPlaintextBuf = folly::IOBuf::copyBuffer(inMessage);
    auto cipherTextBuf = newCipher->encrypt(std::move(inPlaintextBuf)).value();
    auto outPlaintextBuf = newCipher->decrypt(std::move(cipherTextBuf)).value();
    auto outMessage = outPlaintextBuf->moveToFbString().toStdString();
    EXPECT_EQ(inMessage, outMessage);
  }
}

TEST(UtilTest, ReadPKey) {
  {
    folly::test::TemporaryFile testFile("test");
    folly::writeFileAtomic(testFile.path().string(), kP256Key);
    FizzUtil::readPrivateKey(testFile.path().string(), "");
  }
  {
    folly::test::TemporaryFile testFile("test");
    folly::writeFileAtomic(
        testFile.path().string(), folly::StringPiece("test"));
    EXPECT_THROW(
        FizzUtil::readPrivateKey(testFile.path().string(), ""),
        std::runtime_error);
  }
}

TEST(UtilTest, ReadChainFile) {
  {
    folly::test::TemporaryFile testFile("test");
    folly::writeFileAtomic(testFile.path().string(), kP256Certificate);
    EXPECT_EQ(FizzUtil::readChainFile(testFile.path().string()).size(), 1);
  }
  {
    folly::test::TemporaryFile testFile("test");
    folly::writeFileAtomic(testFile.path().string(), kP384Key);
    EXPECT_THROW(
        FizzUtil::readChainFile(testFile.path().string()), std::runtime_error);
  }
  {
    EXPECT_THROW(
        FizzUtil::readChainFile("test_file_does_not_exist"),
        std::runtime_error);
  }
}

TEST(UtilTest, createKeyExchangeFromBuf) {
  {
    // Test X25519 KEM
    auto keys = FizzUtil::generateKeypairCurve25519();
    auto privKey = std::get<0>(keys);
    folly::ByteRange privKeyBuf((folly::StringPiece(privKey)));
    auto kex =
        FizzUtil::createKeyExchangeFromBuf(hpke::KEMId::x25519, privKeyBuf);
    EXPECT_TRUE(kex != nullptr);
    EXPECT_EQ(kex->getKeyShare()->computeChainDataLength(), 32);
  }

  {
    // Test P256 KEM
    folly::ByteRange privKeyBuf((folly::StringPiece(kP256Key)));
    auto kex =
        FizzUtil::createKeyExchangeFromBuf(hpke::KEMId::secp256r1, privKeyBuf);
    EXPECT_TRUE(kex != nullptr);
    EXPECT_EQ(kex->getKeyShare()->computeChainDataLength(), 65);
  }
}

} // namespace test
} // namespace fizz
