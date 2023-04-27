/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/ech/Decrypter.h>
#include <fizz/protocol/ech/test/TestUtil.h>
#include <fizz/protocol/test/TestMessages.h>

using namespace fizz::test;

namespace fizz {
namespace ech {
namespace test {
ClientHello getChloOuterWithExt(std::unique_ptr<KeyExchange> kex) {
  // Setup ECH extension
  auto echConfigContent = getECHConfigContent();
  auto supportedECHConfig = SupportedECHConfig{
      getECHConfig(),
      echConfigContent.key_config.config_id,
      echConfigContent.maximum_name_length,
      HpkeSymmetricCipherSuite{
          hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256}};
  auto setupResult =
      constructHpkeSetupResult(std::move(kex), supportedECHConfig);

  auto chloInner = TestMessages::clientHello();
  InnerECHClientHello chloInnerECHExt;
  chloInner.extensions.push_back(encodeExtension(chloInnerECHExt));

  // Encrypt client hello
  ClientHello chloOuter = getClientHelloOuter();
  chloOuter.legacy_session_id = folly::IOBuf::create(0);

  OuterECHClientHello echExt = encryptClientHello(
      supportedECHConfig, chloInner, chloOuter, setupResult, folly::none);

  // Add ECH extension
  chloOuter.extensions.push_back(encodeExtension(echExt));

  return chloOuter;
}

// Like above, but this handles HRR ECH and also outputs the enc value used
// for the initial round as well as the first outer client hello
ClientHello getChloOuterHRRWithExt(
    std::unique_ptr<KeyExchange> kex,
    Buf& enc,
    ClientHello& initialOuterChlo) {
  // Setup ECH extension
  auto echConfigContent = getECHConfigContent();
  auto supportedECHConfig = SupportedECHConfig{
      getECHConfig(),
      echConfigContent.key_config.config_id,
      echConfigContent.maximum_name_length,
      HpkeSymmetricCipherSuite{
          hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256}};
  auto setupResult =
      constructHpkeSetupResult(std::move(kex), supportedECHConfig);

  auto chloInner = TestMessages::clientHello();
  InnerECHClientHello chloInnerECHExt;
  chloInner.extensions.push_back(encodeExtension(chloInnerECHExt));

  ClientHello chloOuter = getClientHelloOuter();
  chloOuter.legacy_session_id = folly::IOBuf::create(0);

  // Encrypt client hello once to increment counter and get enc value.
  auto initialECH = encryptClientHello(
      supportedECHConfig, chloInner, chloOuter, setupResult, folly::none);

  // First, save out the first ECH
  initialOuterChlo = chloOuter.clone();
  initialOuterChlo.extensions.push_back(encodeExtension(initialECH));

  // Save out enc
  enc = std::move(initialECH.enc);

  // Second encryption for HRR
  OuterECHClientHello echExt = encryptClientHelloHRR(
      supportedECHConfig, chloInner, chloOuter, setupResult, folly::none);

  // Add ECH extension
  chloOuter.extensions.push_back(encodeExtension(echExt));

  return chloOuter;
}

TEST(DecrypterTest, TestDecodeSuccess) {
  auto kex = std::make_unique<OpenSSLECKeyExchange<P256>>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter;
  decrypter.addDecryptionConfig(DecrypterParams{getECHConfig(), kex->clone()});
  auto chloOuter = getChloOuterWithExt(kex->clone());
  auto gotChlo = decrypter.decryptClientHello(chloOuter);

  EXPECT_TRUE(gotChlo.has_value());

  auto expectedChloInner = TestMessages::clientHello();
  EXPECT_FALSE(folly::IOBufEqualTo()(
      encodeHandshake(chloOuter), encodeHandshake(expectedChloInner)));

  auto chlo = std::move(gotChlo.value());
  // Remove the inner ECH extension from the client hello inner
  TestMessages::removeExtension(
      chlo.chlo, ExtensionType::encrypted_client_hello);

  EXPECT_TRUE(folly::IOBufEqualTo()(
      encodeHandshake(chlo.chlo), encodeHandshake(expectedChloInner)));
}

TEST(DecrypterTest, TestDecodeHRRSuccess) {
  auto kex = std::make_unique<OpenSSLECKeyExchange<P256>>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter;
  decrypter.addDecryptionConfig(DecrypterParams{getECHConfig(), kex->clone()});
  Buf enc;
  ClientHello initialChlo;
  auto chloOuter = getChloOuterHRRWithExt(kex->clone(), enc, initialChlo);
  auto echConfigContent = getECHConfigContent();
  auto gotChlo = decrypter.decryptClientHelloHRR(chloOuter, enc);

  auto expectedChloInner = TestMessages::clientHello();
  EXPECT_FALSE(folly::IOBufEqualTo()(
      encodeHandshake(chloOuter), encodeHandshake(expectedChloInner)));

  // Remove the inner ECH extension from the client hello inner
  TestMessages::removeExtension(gotChlo, ExtensionType::encrypted_client_hello);

  EXPECT_TRUE(folly::IOBufEqualTo()(
      encodeHandshake(gotChlo), encodeHandshake(expectedChloInner)));
}

TEST(DecrypterTest, TestDecodeHRRWithContextSuccess) {
  auto kex = std::make_unique<OpenSSLECKeyExchange<P256>>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter;
  decrypter.addDecryptionConfig(DecrypterParams{getECHConfig(), kex->clone()});
  Buf enc;
  ClientHello initialChlo;
  auto chloOuter = getChloOuterHRRWithExt(kex->clone(), enc, initialChlo);

  // Get context from initial chlo
  auto gotChlo = decrypter.decryptClientHello(initialChlo);
  EXPECT_TRUE(gotChlo.has_value());
  auto chlo = std::move(gotChlo.value());

  auto echConfigContent = getECHConfigContent();
  auto gotChloHRR = decrypter.decryptClientHelloHRR(chloOuter, chlo.context);

  auto expectedChloInner = TestMessages::clientHello();
  EXPECT_FALSE(folly::IOBufEqualTo()(
      encodeHandshake(chloOuter), encodeHandshake(expectedChloInner)));

  // Remove the inner ECH extension from the client hello inner
  TestMessages::removeExtension(
      gotChloHRR, ExtensionType::encrypted_client_hello);

  EXPECT_TRUE(folly::IOBufEqualTo()(
      encodeHandshake(gotChloHRR), encodeHandshake(expectedChloInner)));
}

TEST(DecrypterTest, TestDecodeFailure) {
  auto echConfig = getECHConfig();
  auto kex = std::make_unique<OpenSSLECKeyExchange<P256>>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter;
  decrypter.addDecryptionConfig(
      DecrypterParams{std::move(echConfig), kex->clone()});
  auto gotChlo = decrypter.decryptClientHello(TestMessages::clientHello());

  EXPECT_FALSE(gotChlo.has_value());
}

TEST(DecrypterTest, TestDecodeHRRFailure) {
  auto echConfig = getECHConfig();
  auto kex = std::make_unique<OpenSSLECKeyExchange<P256>>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter;
  decrypter.addDecryptionConfig(
      DecrypterParams{std::move(echConfig), kex->clone()});
  // Get an encapsulated key to use.
  Buf enc;
  ClientHello initialClientHello;
  getChloOuterHRRWithExt(kex->clone(), enc, initialClientHello);
  auto echConfigContent = getECHConfigContent();

  EXPECT_THROW(
      decrypter.decryptClientHelloHRR(TestMessages::clientHello(), enc),
      FizzException);
}

TEST(DecrypterTest, TestDecodeHRRWithContextFailure) {
  auto echConfig = getECHConfig();
  auto kex = std::make_unique<OpenSSLECKeyExchange<P256>>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  ECHConfigManager decrypter;
  decrypter.addDecryptionConfig(
      DecrypterParams{std::move(echConfig), kex->clone()});
  // Get a context to use.
  auto chloOuter = getChloOuterWithExt(kex->clone());
  auto gotChlo = decrypter.decryptClientHello(chloOuter);
  auto echConfigContent = getECHConfigContent();

  EXPECT_THROW(
      decrypter.decryptClientHelloHRR(
          TestMessages::clientHello(), gotChlo->context),
      FizzException);
}

} // namespace test
} // namespace ech
} // namespace fizz
