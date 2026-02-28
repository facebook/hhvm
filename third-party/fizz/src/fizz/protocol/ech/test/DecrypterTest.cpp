/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/DefaultFactory.h>
#include <fizz/protocol/ech/Decrypter.h>
#include <fizz/protocol/ech/test/TestUtil.h>
#include <fizz/protocol/test/TestUtil.h>

using namespace std::string_literals;
using namespace fizz::test;

namespace fizz {
namespace ech {
namespace test {
ClientHello getChloOuterWithExt(
    std::unique_ptr<KeyExchange> kex,
    std::optional<ParsedECHConfig> config = std::nullopt) {
  if (!config) {
    config = getParsedECHConfig();
  }

  // Setup ECH extension
  auto configId = config->key_config.config_id;
  auto maxNameLength = config->maximum_name_length;
  auto negotiatedECHConfig = NegotiatedECHConfig{
      std::move(config.value()),
      configId,
      maxNameLength,
      HpkeSymmetricCipherSuite{
          hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256}};
  auto setupResult = constructHpkeSetupResult(
      fizz::DefaultFactory(), std::move(kex), negotiatedECHConfig);

  auto chloInner = TestMessages::clientHello();
  InnerECHClientHello chloInnerECHExt;
  Extension innerECHExt;
  Error err;
  FIZZ_THROW_ON_ERROR(encodeExtension(innerECHExt, err, chloInnerECHExt), err);
  chloInner.extensions.push_back(std::move(innerECHExt));

  // Encrypt client hello
  ClientHello chloOuter = getClientHelloOuter();
  chloOuter.legacy_session_id = folly::IOBuf::create(0);

  OuterECHClientHello echExt = encryptClientHello(
      negotiatedECHConfig, chloInner, chloOuter, setupResult, folly::none, {});

  // Add ECH extension
  Extension outerECHExt;
  FIZZ_THROW_ON_ERROR(encodeExtension(outerECHExt, err, echExt), err);
  chloOuter.extensions.push_back(std::move(outerECHExt));

  return chloOuter;
}

// Like above, but this handles HRR ECH and also outputs the enc value used
// for the initial round as well as the first outer client hello
ClientHello getChloOuterHRRWithExt(
    std::unique_ptr<KeyExchange> kex,
    Buf& enc,
    ClientHello& initialOuterChlo) {
  // Setup ECH extension
  auto echConfigContent = getParsedECHConfig();
  auto negotiatedECHConfig = NegotiatedECHConfig{
      getParsedECHConfig(),
      echConfigContent.key_config.config_id,
      echConfigContent.maximum_name_length,
      HpkeSymmetricCipherSuite{
          hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256}};
  auto setupResult = constructHpkeSetupResult(
      fizz::DefaultFactory(), std::move(kex), negotiatedECHConfig);

  auto chloInner = TestMessages::clientHello();
  InnerECHClientHello chloInnerECHExt;
  Extension innerECHExt;
  Error err;
  FIZZ_THROW_ON_ERROR(encodeExtension(innerECHExt, err, chloInnerECHExt), err);
  chloInner.extensions.push_back(std::move(innerECHExt));

  ClientHello chloOuter = getClientHelloOuter();
  chloOuter.legacy_session_id = folly::IOBuf::create(0);

  // Encrypt client hello once to increment counter and get enc value.
  auto initialECH = encryptClientHello(
      negotiatedECHConfig, chloInner, chloOuter, setupResult, folly::none, {});

  // First, save out the first ECH
  initialOuterChlo = chloOuter.clone();
  Extension initialECHExt;
  FIZZ_THROW_ON_ERROR(encodeExtension(initialECHExt, err, initialECH), err);
  initialOuterChlo.extensions.push_back(std::move(initialECHExt));

  // Save out enc
  enc = std::move(initialECH.enc);

  // Second encryption for HRR
  OuterECHClientHello echExt = encryptClientHelloHRR(
      negotiatedECHConfig, chloInner, chloOuter, setupResult, folly::none, {});

  // Add ECH extension
  Extension outerECHExt;
  FIZZ_THROW_ON_ERROR(encodeExtension(outerECHExt, err, echExt), err);
  chloOuter.extensions.push_back(std::move(outerECHExt));

  return chloOuter;
}

ParsedECHConfig makeDummyConfig(
    uint8_t configId,
    const std::string& publicName) {
  auto config = ParsedECHConfig{};
  config.key_config.config_id = configId;
  config.public_name = publicName;
  config.key_config.public_key = folly::IOBuf::copyBuffer("public");
  return config;
}

struct RetryConfigExpectation {
  uint8_t id;
  std::string name;
};

void checkRetryConfigExpectation(
    const std::vector<RetryConfigExpectation>& expectations,
    const std::vector<ECHConfig>& configs) {
  ASSERT_EQ(expectations.size(), configs.size());
  for (size_t i = 0; i < expectations.size(); ++i) {
    auto config = ParsedECHConfig::parseSupportedECHConfig(configs[i]);
    ASSERT_TRUE(config.hasValue());
    EXPECT_EQ(expectations[i].id, config->key_config.config_id);
    EXPECT_EQ(expectations[i].name, config->public_name);
  }
}

TEST(DecrypterTest, TestDecodeSuccess) {
  auto kex = openssl::makeOpenSSLECKeyExchange<fizz::P256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter(std::make_shared<fizz::DefaultFactory>());
  decrypter.addDecryptionConfig(
      DecrypterParams{getParsedECHConfig(), kex->clone()});
  auto chloOuter = getChloOuterWithExt(kex->clone());
  auto gotChlo = decrypter.decryptClientHello(chloOuter);

  EXPECT_TRUE(gotChlo.has_value());

  auto expectedChloInner = TestMessages::clientHello();
  Buf encodedOuter;
  Error err;
  EXPECT_EQ(encodeHandshake(encodedOuter, err, chloOuter), Status::Success);
  Buf encodedExpectedOuter;
  EXPECT_EQ(
      encodeHandshake(encodedExpectedOuter, err, expectedChloInner),
      Status::Success);
  EXPECT_FALSE(
      folly::IOBufEqualTo()(
          std::move(encodedOuter), std::move(encodedExpectedOuter)));

  auto chlo = std::move(gotChlo.value());
  // Remove the inner ECH extension from the client hello inner
  TestMessages::removeExtension(
      chlo.chlo, ExtensionType::encrypted_client_hello);

  Buf encodedChlo;
  EXPECT_EQ(encodeHandshake(encodedChlo, err, chlo.chlo), Status::Success);
  Buf encodedExpectedInner;
  EXPECT_EQ(
      encodeHandshake(encodedExpectedInner, err, expectedChloInner),
      Status::Success);
  EXPECT_TRUE(
      folly::IOBufEqualTo()(
          std::move(encodedChlo), std::move(encodedExpectedInner)));
}

TEST(DecrypterTest, TestDecodeHRRSuccess) {
  auto kex = openssl::makeOpenSSLECKeyExchange<fizz::P256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter(std::make_shared<fizz::DefaultFactory>());
  decrypter.addDecryptionConfig(
      DecrypterParams{getParsedECHConfig(), kex->clone()});
  Buf enc;
  ClientHello initialChlo;
  auto chloOuter = getChloOuterHRRWithExt(kex->clone(), enc, initialChlo);
  auto gotChlo = decrypter.decryptClientHelloHRR(chloOuter, enc);

  auto expectedChloInner = TestMessages::clientHello();
  Error err;
  {
    Buf encodedOuter;
    EXPECT_EQ(encodeHandshake(encodedOuter, err, chloOuter), Status::Success);
    Buf encodedExpectedInner;
    EXPECT_EQ(
        encodeHandshake(encodedExpectedInner, err, expectedChloInner),
        Status::Success);
    EXPECT_FALSE(
        folly::IOBufEqualTo()(
            std::move(encodedOuter), std::move(encodedExpectedInner)));
  }
  // Remove the inner ECH extension from the client hello inner
  TestMessages::removeExtension(gotChlo, ExtensionType::encrypted_client_hello);
  {
    Buf encodedGotChloHRR;
    EXPECT_EQ(
        encodeHandshake(encodedGotChloHRR, err, gotChlo), Status::Success);
    Buf encodedExpectedInner;
    EXPECT_EQ(
        encodeHandshake(encodedExpectedInner, err, expectedChloInner),
        Status::Success);
    EXPECT_TRUE(
        folly::IOBufEqualTo()(
            std::move(encodedGotChloHRR), std::move(encodedExpectedInner)));
  }
}

TEST(DecrypterTest, TestDecodeHRRWithContextSuccess) {
  auto kex = openssl::makeOpenSSLECKeyExchange<fizz::P256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter(std::make_shared<fizz::DefaultFactory>());
  decrypter.addDecryptionConfig(
      DecrypterParams{getParsedECHConfig(), kex->clone()});
  Buf enc;
  ClientHello initialChlo;
  auto chloOuter = getChloOuterHRRWithExt(kex->clone(), enc, initialChlo);

  // Get context from initial chlo
  auto gotChlo = decrypter.decryptClientHello(initialChlo);
  EXPECT_TRUE(gotChlo.has_value());
  auto chlo = std::move(gotChlo.value());

  auto gotChloHRR = decrypter.decryptClientHelloHRR(chloOuter, chlo.context);

  auto expectedChloInner = TestMessages::clientHello();
  Error err;
  {
    Buf encodedOuter;
    EXPECT_EQ(encodeHandshake(encodedOuter, err, chloOuter), Status::Success);
    Buf encodedExpectedInner;
    EXPECT_EQ(
        encodeHandshake(encodedExpectedInner, err, expectedChloInner),
        Status::Success);
    EXPECT_FALSE(
        folly::IOBufEqualTo()(
            std::move(encodedOuter), std::move(encodedExpectedInner)));
  }
  // Remove the inner ECH extension from the client hello inner
  TestMessages::removeExtension(
      gotChloHRR, ExtensionType::encrypted_client_hello);
  {
    Buf encodedGotChloHRR;
    EXPECT_EQ(
        encodeHandshake(encodedGotChloHRR, err, gotChloHRR), Status::Success);
    Buf encodedExpectedInner;
    EXPECT_EQ(
        encodeHandshake(encodedExpectedInner, err, expectedChloInner),
        Status::Success);
    EXPECT_TRUE(
        folly::IOBufEqualTo()(
            std::move(encodedGotChloHRR), std::move(encodedExpectedInner)));
  }
}

TEST(DecrypterTest, TestDecodeFailure) {
  auto kex = openssl::makeOpenSSLECKeyExchange<fizz::P256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter(std::make_shared<fizz::DefaultFactory>());
  decrypter.addDecryptionConfig(
      DecrypterParams{getParsedECHConfig(), kex->clone()});
  auto gotChlo = decrypter.decryptClientHello(TestMessages::clientHello());

  EXPECT_FALSE(gotChlo.has_value());
}

TEST(DecrypterTest, TestDecodeHRRFailure) {
  auto kex = openssl::makeOpenSSLECKeyExchange<fizz::P256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter(std::make_shared<fizz::DefaultFactory>());
  decrypter.addDecryptionConfig(
      DecrypterParams{getParsedECHConfig(), kex->clone()});
  // Get an encapsulated key to use.
  Buf enc;
  ClientHello initialClientHello;
  getChloOuterHRRWithExt(kex->clone(), enc, initialClientHello);

  EXPECT_THROW(
      decrypter.decryptClientHelloHRR(TestMessages::clientHello(), enc),
      FizzException);
}

TEST(DecrypterTest, TestDecodeHRRWithContextFailure) {
  auto kex = openssl::makeOpenSSLECKeyExchange<fizz::P256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter(std::make_shared<fizz::DefaultFactory>());
  decrypter.addDecryptionConfig(
      DecrypterParams{getParsedECHConfig(), kex->clone()});
  // Get a context to use.
  auto chloOuter = getChloOuterWithExt(kex->clone());
  auto gotChlo = decrypter.decryptClientHello(chloOuter);

  EXPECT_THROW(
      decrypter.decryptClientHelloHRR(
          TestMessages::clientHello(), gotChlo->context),
      FizzException);
}

TEST(DecrypterTest, TestDecodeMultipleDecrypterParam) {
  auto targetECHConfigContent = getParsedECHConfig();
  targetECHConfigContent.key_config.config_id = 2;

  auto kex = openssl::makeOpenSSLECKeyExchange<fizz::P256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));
  ECHConfigManager decrypter(std::make_shared<fizz::DefaultFactory>());
  // Load multiple decrypter params
  decrypter.addDecryptionConfig(
      DecrypterParams{getParsedECHConfig(), kex->clone()});
  decrypter.addDecryptionConfig(
      DecrypterParams{targetECHConfigContent, kex->clone()});

  Buf enc;
  auto chlo = getChloOuterWithExt(kex->clone(), targetECHConfigContent);

  // Decrypting chlo should result in the second decrypter param being used
  auto result = decrypter.decryptClientHello(chlo);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->configId == 2);
}

TEST(DecrypterTest, TestGetRetryConfigs) {
  auto decrypter = ECHConfigManager(std::make_shared<DefaultFactory>());
  auto publicNames = {"a.com", "b.com", "a.com", "c.com", "b.com", "a.com"};
  uint8_t configId = 0;
  for (const auto& name : publicNames) {
    decrypter.addDecryptionConfig(
        {.echConfig = makeDummyConfig(configId++, name)});
  }

  checkRetryConfigExpectation(
      {{0, "a.com"},
       {2, "a.com"},
       {5, "a.com"},
       {1, "b.com"},
       {3, "c.com"},
       {4, "b.com"}},
      decrypter.getRetryConfigs("a.com"s));
  checkRetryConfigExpectation(
      {{1, "b.com"},
       {4, "b.com"},
       {0, "a.com"},
       {2, "a.com"},
       {3, "c.com"},
       {5, "a.com"}},
      decrypter.getRetryConfigs("b.com"s));
  checkRetryConfigExpectation(
      {{3, "c.com"},
       {0, "a.com"},
       {1, "b.com"},
       {2, "a.com"},
       {4, "b.com"},
       {5, "a.com"}},
      decrypter.getRetryConfigs("c.com"s));
  checkRetryConfigExpectation(
      {{0, "a.com"},
       {1, "b.com"},
       {2, "a.com"},
       {3, "c.com"},
       {4, "b.com"},
       {5, "a.com"}},
      decrypter.getRetryConfigs("d.com"s));
  checkRetryConfigExpectation(
      {{0, "a.com"},
       {1, "b.com"},
       {2, "a.com"},
       {3, "c.com"},
       {4, "b.com"},
       {5, "a.com"}},
      decrypter.getRetryConfigs(folly::none));
}

} // namespace test
} // namespace ech
} // namespace fizz
