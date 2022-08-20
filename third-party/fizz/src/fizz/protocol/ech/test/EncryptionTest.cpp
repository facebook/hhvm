/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <fizz/crypto/test/TestUtil.h>

#include <fizz/crypto/hpke/Hpke.h>
#include <fizz/crypto/hpke/Utils.h>
#include <fizz/protocol/ech/Encryption.h>
#include <fizz/protocol/ech/test/TestUtil.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/protocol/test/TestMessages.h>
#include <fizz/record/Extensions.h>
#include <folly/lang/Bits.h>

using namespace fizz::test;

namespace fizz {
namespace ech {
namespace test {

namespace {
static constexpr folly::StringPiece testLegacySessionId{
    "test legacy session id"};
static std::vector<hpke::KEMId> supportedKEMs{
    hpke::KEMId::x25519,
    hpke::KEMId::secp256r1};
static std::vector<hpke::AeadId> supportedAeads{
    hpke::AeadId::TLS_AES_256_GCM_SHA384,
    hpke::AeadId::TLS_AES_128_GCM_SHA256};

class MockOpenSSLECKeyExchange256 : public OpenSSLECKeyExchange<P256> {
 public:
  MOCK_METHOD(void, generateKeyPair, ());
};

void checkDecodedChlo(ClientHello decodedChlo, ClientHello expectedChlo) {
  EXPECT_TRUE(folly::IOBufEqualTo()(
      decodedChlo.legacy_session_id, expectedChlo.legacy_session_id));
  EXPECT_EQ(decodedChlo.extensions.size(), expectedChlo.extensions.size());
  for (size_t extIndex = 0; extIndex < decodedChlo.extensions.size();
       ++extIndex) {
    EXPECT_TRUE(folly::IOBufEqualTo()(
        decodedChlo.extensions[extIndex].extension_data,
        expectedChlo.extensions[extIndex].extension_data));
  }
  EXPECT_EQ(decodedChlo.random, expectedChlo.random);
  EXPECT_EQ(decodedChlo.cipher_suites, expectedChlo.cipher_suites);
  EXPECT_EQ(
      decodedChlo.legacy_compression_methods,
      expectedChlo.legacy_compression_methods);
}

ECHConfig getInvalidVECHConfig() {
  // Add invalid config
  ECHConfig invalidConfig;
  invalidConfig.version = ECHVersion::Draft9;
  auto configContent = getECHConfigContent();
  configContent.kem_id = hpke::KEMId::secp384r1;
  invalidConfig.ech_config_content = encode(std::move(configContent));

  return invalidConfig;
}

hpke::SetupResult constructSetupResult(
    const SupportedECHConfig& supportedConfig) {
  auto kex = std::make_unique<MockOpenSSLECKeyExchange256>();
  auto privateKey = getPrivateKey(kP256Key);
  kex->setPrivateKey(std::move(privateKey));
  EXPECT_CALL(*kex, generateKeyPair()).Times(1);

  return constructHpkeSetupResult(std::move(kex), supportedConfig);
}

ClientECH getTestClientECHWithInner(ClientHello chloInner) {
  SupportedECHConfig supportedConfig{
      getECHConfig(),
      ECHCipherSuite{
          hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256}};

  auto setupResult = constructSetupResult(supportedConfig);
  chloInner.legacy_session_id = folly::IOBuf::copyBuffer(testLegacySessionId);

  return encryptClientHello(
      supportedConfig,
      std::move(chloInner),
      getClientHelloOuter(),
      setupResult);
}

ClientECH getTestClientECH() {
  return getTestClientECHWithInner(TestMessages::clientHello());
}

void checkSupportedConfigValid(
    const std::vector<ECHConfig>& configs,
    std::unique_ptr<folly::IOBuf> expectedECHConfigContent) {
  folly::Optional<SupportedECHConfig> result =
      selectECHConfig(configs, supportedKEMs, supportedAeads);
  EXPECT_TRUE(result.hasValue());

  ECHConfig gotConfig = std::move(result.value().config);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      gotConfig.ech_config_content, expectedECHConfigContent));
  EXPECT_EQ(result.value().cipherSuite.kdf_id, hpke::KDFId::Sha256);
  EXPECT_EQ(
      result.value().cipherSuite.aead_id, hpke::AeadId::TLS_AES_128_GCM_SHA256);
}

} // namespace

TEST(EncryptionTest, TestValidECHConfigContent) {
  // Add config that doesn't work and cannot be supported
  ECHConfigContentDraft invalidConfigContent = getECHConfigContent();
  invalidConfigContent.kem_id = hpke::KEMId::secp521r1;
  std::vector<ECHConfig> configs;
  ECHConfig invalid;
  invalid.version = ECHVersion::Draft9;
  invalid.ech_config_content = encode(std::move(invalidConfigContent));

  // Add config that works and can be supported
  ECHConfig valid = getECHConfig();

  configs.push_back(std::move(invalid));
  configs.push_back(std::move(valid));

  checkSupportedConfigValid(configs, encode(getECHConfigContent()));
}

TEST(EncryptionTest, TestInvalidECHConfigContent) {
  ECHConfigContentDraft configContent = getECHConfigContent();

  configContent.kem_id = hpke::KEMId::secp256r1;
  ECHCipherSuite suite{
      hpke::KDFId::Sha512, hpke::AeadId::TLS_AES_128_GCM_SHA256};
  std::vector<ECHCipherSuite> cipher_suites = {suite};
  configContent.cipher_suites = cipher_suites;

  ECHConfig invalidConfig;
  invalidConfig.version = static_cast<ECHVersion>(0xfe07); // Draft 7
  invalidConfig.ech_config_content = encode(std::move(configContent));

  std::vector<ECHConfig> configs;
  configs.push_back(std::move(invalidConfig));

  folly::Optional<SupportedECHConfig> result =
      selectECHConfig(configs, supportedKEMs, supportedAeads);

  EXPECT_FALSE(result.hasValue());
}

TEST(EncryptionTest, TestUnsupportedMandatoryExtension) {
  // Add config that would work save for a mandatory extension we don't support.
  ECHConfigContentDraft invalidConfigContent = getECHConfigContent();

  Extension mandatory;
  // Set high order bit for type
  mandatory.extension_type =
      static_cast<ExtensionType>(1 << ((sizeof(uint16_t) * 8) - 1));
  mandatory.extension_data = folly::IOBuf::create(0);
  invalidConfigContent.extensions.clear();
  invalidConfigContent.extensions.push_back(std::move(mandatory));

  std::vector<ECHConfig> configs;
  ECHConfig invalid;
  invalid.version = ECHVersion::Draft9;
  invalid.ech_config_content = encode(std::move(invalidConfigContent));
  configs.push_back(std::move(invalid));

  folly::Optional<SupportedECHConfig> result =
      selectECHConfig(configs, supportedKEMs, supportedAeads);

  // Expect no result thanks to mandatory extension.
  EXPECT_FALSE(result.hasValue());
}

TEST(EncryptionTest, TestValidSelectECHConfigContent) {
  // Add valid config
  ECHConfig validConfig;
  validConfig.version = ECHVersion::Draft9;
  validConfig.ech_config_content = encode(getECHConfigContent());

  std::vector<ECHConfig> configs;
  configs.push_back(getInvalidVECHConfig());
  configs.push_back(std::move(validConfig));

  checkSupportedConfigValid(configs, encode(getECHConfigContent()));
}

TEST(EncryptionTest, TestInvalidSelectECHConfigContent) {
  std::vector<ECHConfig> configs;
  configs.push_back(getInvalidVECHConfig());

  folly::Optional<SupportedECHConfig> result =
      selectECHConfig(configs, supportedKEMs, supportedAeads);

  EXPECT_FALSE(result.hasValue());
}

TEST(EncryptionTest, TestValidEncryptClientHello) {
  auto clientECH = getTestClientECH();
  auto expectedChlo = TestMessages::clientHello();
  // Add a legacy_session_id to match client hello inner used in
  // getTestClientECH()
  expectedChlo.legacy_session_id =
      folly::IOBuf::copyBuffer("test legacy session id");

  // Create HPKE setup prefix
  std::string tlsEchPrefix = "tls ech";
  tlsEchPrefix += '\0';
  auto hpkePrefix = folly::IOBuf::copyBuffer(tlsEchPrefix);
  hpkePrefix->prependChain(encode(getECHConfig()));

  const std::unique_ptr<folly::IOBuf> prefix =
      folly::IOBuf::copyBuffer("HPKE-07");

  auto kex = std::make_unique<MockOpenSSLECKeyExchange256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  auto suiteId = hpke::generateHpkeSuiteId(
      NamedGroup::secp256r1,
      HashFunction::Sha256,
      CipherSuite::TLS_AES_128_GCM_SHA256);

  auto kdfId = hpke::getKDFId(HashFunction::Sha256);

  auto dhkem = std::make_unique<DHKEM>(
      std::move(kex),
      NamedGroup::secp256r1,
      hpke::makeHpkeHkdf(prefix->clone(), kdfId));

  hpke::SetupParam setupParam{
      std::move(dhkem),
      makeCipher(hpke::AeadId::TLS_AES_128_GCM_SHA256),
      hpke::makeHpkeHkdf(prefix->clone(), kdfId),
      std::move(suiteId),
  };

  auto context = setupWithDecap(
      hpke::Mode::Base,
      clientECH.enc->coalesce(),
      std::move(hpkePrefix),
      folly::none,
      std::move(setupParam));

  // Get client hello inner by decrypting
  auto clientHelloOuterEnc = encode(getClientHelloOuter());
  auto clientHelloOuterAad = makeClientHelloAad(
      clientECH.cipher_suite,
      clientECH.config_id,
      clientECH.enc,
      clientHelloOuterEnc);
  std::unique_ptr<folly::IOBuf> gotClientHelloInner =
      context->open(clientHelloOuterAad.get(), std::move(clientECH.payload));

  folly::io::Cursor encodedECHInnerCursor(gotClientHelloInner.get());
  auto gotChlo = decode<ClientHello>(encodedECHInnerCursor);

  // Check that we don't have a legacy_session_id (it should have gotten removed
  // during encryption)
  EXPECT_TRUE(folly::IOBufEqualTo()(
      gotChlo.legacy_session_id, folly::IOBuf::copyBuffer("")));

  // Replace legacy_session_id that was removed during encryption
  gotChlo.legacy_session_id =
      folly::IOBuf::copyBuffer("test legacy session id");
  checkDecodedChlo(std::move(gotChlo), std::move(expectedChlo));
}

TEST(EncryptionTest, TestTryToDecryptECH) {
  auto expectedChlo = TestMessages::clientHello();
  expectedChlo.legacy_session_id =
      folly::IOBuf::copyBuffer("test legacy session id");

  // Add ECH extension to client hello outer.
  auto chloOuter = getClientHelloOuter();
  auto testECH = getTestClientECH();
  chloOuter.extensions.push_back(encodeExtension(testECH));

  auto kex = std::make_unique<MockOpenSSLECKeyExchange256>();
  auto privateKey = getPrivateKey(kP256Key);
  kex->setPrivateKey(std::move(privateKey));

  auto context = setupDecryptionContext(
      getECHConfig(),
      testECH.cipher_suite,
      testECH.enc->clone(),
      std::move(kex),
      0);

  auto chlo = decryptECHWithContext(
      chloOuter,
      getECHConfig(),
      testECH.cipher_suite,
      std::move(testECH.enc),
      std::move(testECH.config_id),
      std::move(testECH.payload),
      ECHVersion::Draft9,
      context);

  checkDecodedChlo(std::move(chlo), std::move(expectedChlo));
}

TEST(EncryptionTest, TestInnerClientHelloOuterExtensionsContainsECH) {
  auto innerChlo = TestMessages::clientHello();
  // Add OuterExtensions with ECH extension. Should blow up on decrypt.
  OuterExtensions outer;
  outer.types = {ExtensionType::encrypted_client_hello};
  innerChlo.extensions.push_back(encodeExtension(std::move(outer)));
  auto clientECH = getTestClientECHWithInner(std::move(innerChlo));

  // Create HPKE setup prefix
  std::string tlsEchPrefix = "tls ech";
  tlsEchPrefix += '\0';
  auto hpkePrefix = folly::IOBuf::copyBuffer(tlsEchPrefix);
  hpkePrefix->prependChain(encode(getECHConfig()));

  const std::unique_ptr<folly::IOBuf> prefix =
      folly::IOBuf::copyBuffer("HPKE-07");

  auto kex = std::make_unique<MockOpenSSLECKeyExchange256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  auto suiteId = hpke::generateHpkeSuiteId(
      NamedGroup::secp256r1,
      HashFunction::Sha256,
      CipherSuite::TLS_AES_128_GCM_SHA256);

  auto kdfId = hpke::getKDFId(HashFunction::Sha256);

  auto dhkem = std::make_unique<DHKEM>(
      std::move(kex),
      NamedGroup::secp256r1,
      hpke::makeHpkeHkdf(prefix->clone(), kdfId));

  hpke::SetupParam setupParam{
      std::move(dhkem),
      makeCipher(hpke::AeadId::TLS_AES_128_GCM_SHA256),
      hpke::makeHpkeHkdf(prefix->clone(), kdfId),
      std::move(suiteId),
  };

  auto context = setupWithDecap(
      hpke::Mode::Base,
      clientECH.enc->coalesce(),
      std::move(hpkePrefix),
      folly::none,
      std::move(setupParam));
  auto outerChlo = getClientHelloOuter();
  outerChlo.extensions.push_back(encodeExtension(clientECH));

  EXPECT_THROW(
      decryptECHWithContext(
          outerChlo,
          getECHConfig(),
          clientECH.cipher_suite,
          std::move(clientECH.enc),
          std::move(clientECH.config_id),
          std::move(clientECH.payload),
          ECHVersion::Draft9,
          context),
      OuterExtensionsError);
}

TEST(EncryptionTest, TestInnerClientHelloOuterExtensionsContainsDupes) {
  auto innerChlo = TestMessages::clientHello();
  // Add OuterExtensions with SNI extension. Should blow up on decrypt.
  OuterExtensions outer;
  outer.types = {ExtensionType::server_name};
  innerChlo.extensions.push_back(encodeExtension(std::move(outer)));
  auto clientECH = getTestClientECHWithInner(std::move(innerChlo));

  // Create HPKE setup prefix
  std::string tlsEchPrefix = "tls ech";
  tlsEchPrefix += '\0';
  auto hpkePrefix = folly::IOBuf::copyBuffer(tlsEchPrefix);
  hpkePrefix->prependChain(encode(getECHConfig()));

  const std::unique_ptr<folly::IOBuf> prefix =
      folly::IOBuf::copyBuffer("HPKE-07");

  auto kex = std::make_unique<MockOpenSSLECKeyExchange256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  auto suiteId = hpke::generateHpkeSuiteId(
      NamedGroup::secp256r1,
      HashFunction::Sha256,
      CipherSuite::TLS_AES_128_GCM_SHA256);

  auto kdfId = hpke::getKDFId(HashFunction::Sha256);

  auto dhkem = std::make_unique<DHKEM>(
      std::move(kex),
      NamedGroup::secp256r1,
      hpke::makeHpkeHkdf(prefix->clone(), kdfId));

  hpke::SetupParam setupParam{
      std::move(dhkem),
      makeCipher(hpke::AeadId::TLS_AES_128_GCM_SHA256),
      hpke::makeHpkeHkdf(prefix->clone(), kdfId),
      std::move(suiteId),
  };

  auto context = setupWithDecap(
      hpke::Mode::Base,
      clientECH.enc->coalesce(),
      std::move(hpkePrefix),
      folly::none,
      std::move(setupParam));
  auto outerChlo = getClientHelloOuter();
  outerChlo.extensions.push_back(encodeExtension(clientECH));

  EXPECT_THROW(
      decryptECHWithContext(
          outerChlo,
          getECHConfig(),
          clientECH.cipher_suite,
          std::move(clientECH.enc),
          std::move(clientECH.config_id),
          std::move(clientECH.payload),
          ECHVersion::Draft9,
          context),
      OuterExtensionsError);
}

MATCHER_P(ExtensionEq, expectedVec, "") {
  return std::equal(
      arg.get().begin(),
      arg.get().end(),
      expectedVec.begin(),
      [](const Extension& a, const Extension* b) {
        return a.extension_type == b->extension_type &&
            folly::IOBufEqualTo{}(a.extension_data, b->extension_data);
      });
}

static std::vector<Extension> cloneExtensionList(std::vector<Extension*> list) {
  std::vector<Extension> res;
  for (auto ext : list) {
    res.push_back(ext->clone());
  }

  return res;
}

TEST(EncryptionTest, TestSubstituteOuterExtensions) {
  auto innerChlo = TestMessages::clientHello();
  auto outerChlo = TestMessages::clientHello();

  auto& outerExt = outerChlo.extensions;
  EchOuterExtensions echOuterExt;
  std::vector<Extension*> expectedRes;

  // This should not be permitted to be copied out.
  Extension echExt = {
      ExtensionType::encrypted_client_hello, folly::IOBuf::create(0)};

  Extension extA = outerExt.at(0).clone(), extB = outerExt.at(1).clone(),
            extC = outerExt.at(2).clone(), extD = outerExt.at(3).clone(),
            extE = outerExt.at(4).clone(), extF = outerExt.at(5).clone(),
            extG = outerExt.at(6).clone();

  /*
   * If innerClientHello does not have the ech_outer_extensions extension
   * included, we expect a list identical to innerExt.
   *
   * outerExt: []
   * innerExt: [A, B, C, D, E, F, G]
   *
   * result: [A, B, C, D, E, F, G]
   */

  auto actualRes =
      substituteOuterExtensions(std::move(innerChlo.extensions), {});

  expectedRes = {&extA, &extB, &extC, &extD, &extE, &extF, &extG};

  EXPECT_THAT(std::ref(actualRes), ExtensionEq(expectedRes));

  /*
   * If the ech_outer_extensions includes an extension_type that is not
   * present in the outerClientHello, we expect it to throw an exception.
   *
   * outerExt: []
   * innerExt: [A, B, C, D, E, F, G, outer_extensions(H)]
   *
   * result: OuterExtensionsError
   */
  innerChlo = TestMessages::clientHello();

  echOuterExt.extensionTypes.push_back(ExtensionType::early_data);
  innerChlo.extensions.push_back(encodeExtension(echOuterExt));

  EXPECT_THROW(
      substituteOuterExtensions(std::move(innerChlo.extensions), {}),
      OuterExtensionsError);

  /**
   * If the ech_outer_extensions includes extension_type values that do not
   * maintain its relative ordering found in outerClientHello, we expect it to
   * throw an exception.
   *
   * outerExt: [E, F, G]
   * innerExt: [outer_extensions(E, G, F)]
   *
   * result: OuterExtensionsError
   */

  // return values to default
  innerChlo = TestMessages::clientHello();
  innerChlo.extensions.clear();

  outerExt = cloneExtensionList({&extE, &extF, &extG});

  echOuterExt.extensionTypes = {
      extE.extension_type, extG.extension_type, extF.extension_type};

  innerChlo.extensions.push_back(encodeExtension(echOuterExt));

  EXPECT_THROW(
      substituteOuterExtensions(std::move(innerChlo.extensions), outerExt),
      OuterExtensionsError);

  /**
   * If the ech_outer_extensions does not match a contiguous set of extensions
   * in the outerClientHello, it should not fail.
   *
   * outerExt: [D, E, F, G]
   * innerExt: [outer_extensions(D, F, G)]
   *
   * result: [D, F, G]
   */

  // return values to default
  innerChlo = TestMessages::clientHello();
  innerChlo.extensions.clear();

  outerExt = cloneExtensionList({&extD, &extE, &extF, &extG});

  echOuterExt.extensionTypes = {
      extD.extension_type, extF.extension_type, extG.extension_type};

  innerChlo.extensions.push_back(encodeExtension(echOuterExt));

  actualRes =
      substituteOuterExtensions(std::move(innerChlo.extensions), outerExt);
  expectedRes = {&extD, &extF, &extG};

  EXPECT_THAT(std::ref(actualRes), ExtensionEq(expectedRes));

  /*
   * If all ech_outer_extensions values are all present in outerExt and maintain
   * its relative ordering, we expect the returned list's to be equivalent to
   * innerExt with ech_outer_extensions expanded.
   *
   * outerExt: [D, E, F]
   * innerExt: [A, B, C, outer_extensions(D, E, F), G]
   *
   * result: [A, B, C, D, E, F, G]
   */
  innerChlo = TestMessages::clientHello();

  outerExt = cloneExtensionList({&extD, &extE, &extF});

  echOuterExt.extensionTypes = {
      extD.extension_type, extE.extension_type, extF.extension_type};

  auto encodedEchOuterExt = encodeExtension(echOuterExt);
  innerChlo.extensions =
      cloneExtensionList({&extA, &extB, &extC, &encodedEchOuterExt, &extG});

  actualRes =
      substituteOuterExtensions(std::move(innerChlo.extensions), outerExt);

  expectedRes = {&extA, &extB, &extC, &extD, &extE, &extF, &extG};

  EXPECT_THAT(std::ref(actualRes), ExtensionEq(expectedRes));

  /*
   * If the inner client hello has duplicate extensions, we expect it to throw
   * an error.
   *
   * outerExt: [A, B, C]
   * innerExt: [D, E, F, F]
   *
   * result: OuterExtensionsError
   */

  innerChlo = TestMessages::clientHello();

  outerExt = cloneExtensionList({&extA, &extB, &extC});

  innerChlo.extensions = cloneExtensionList({&extD, &extE, &extF, &extF});

  EXPECT_THROW(
      substituteOuterExtensions(std::move(innerChlo.extensions), outerExt),
      OuterExtensionsError);

  /*
   * If the expanded inner client hello has duplicate extensions, we expect it
   * to throw an error.
   *
   * outerExt: [A, B, C, D]
   * innerExt: [D, E, F, outer_extensions(D)]
   *
   * result: OuterExtensionsError
   */
  innerChlo = TestMessages::clientHello();

  outerExt = cloneExtensionList({&extA, &extB, &extC, &extD});

  echOuterExt.extensionTypes = {extD.extension_type};

  encodedEchOuterExt = encodeExtension(echOuterExt);

  innerChlo.extensions =
      cloneExtensionList({&extD, &extE, &extF, &encodedEchOuterExt});

  EXPECT_THROW(
      substituteOuterExtensions(std::move(innerChlo.extensions), outerExt),
      OuterExtensionsError);

  /*
   * If we attempt to copy out the ECH extension to the innerExt, we expect it
   * to throw an error.
   *
   * outerExt: [ECH, A]
   * innerExt: [outer_extensions(ECH), B]
   *
   * result: OuterExtensionsError
   */
  innerChlo = TestMessages::clientHello();

  outerExt = cloneExtensionList({&echExt, &extA});

  echOuterExt.extensionTypes = {echExt.extension_type};

  encodedEchOuterExt = encodeExtension(echOuterExt);

  innerChlo.extensions = cloneExtensionList({&encodedEchOuterExt, &extB});

  EXPECT_THROW(
      substituteOuterExtensions(std::move(innerChlo.extensions), outerExt),
      OuterExtensionsError);
}

} // namespace test
} // namespace ech
} // namespace fizz
