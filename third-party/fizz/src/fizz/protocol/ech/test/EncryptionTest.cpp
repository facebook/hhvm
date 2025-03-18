/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <fizz/crypto/test/TestUtil.h>

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/backend/openssl/crypto/ECCurve.h>
#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/hpke/Hpke.h>
#include <fizz/crypto/hpke/Utils.h>
#include <fizz/protocol/DefaultFactory.h>
#include <fizz/protocol/ech/Encryption.h>
#include <fizz/protocol/ech/test/TestUtil.h>
#include <fizz/protocol/test/Matchers.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/protocol/test/TestUtil.h>
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

class MockOpenSSLECKeyExchange256 : public openssl::OpenSSLECKeyExchange {
 public:
  MockOpenSSLECKeyExchange256()
      : openssl::OpenSSLECKeyExchange(
            openssl::Properties<fizz::P256>::curveNid,
            fizz::P256::keyShareLength) {}

  ~MockOpenSSLECKeyExchange256() = default;

  MOCK_METHOD(void, generateKeyPair, ());
};

void checkExtensions(
    const std::vector<Extension>& testExts,
    const std::vector<Extension>& expectedExts) {
  EXPECT_EQ(testExts.size(), expectedExts.size());
  for (size_t extIndex = 0; extIndex < testExts.size(); ++extIndex) {
    EXPECT_TRUE(folly::IOBufEqualTo()(
        testExts[extIndex].extension_data,
        expectedExts[extIndex].extension_data));
  }
}

void checkDecodedChlo(ClientHello decodedChlo, ClientHello expectedChlo) {
  EXPECT_TRUE(folly::IOBufEqualTo()(
      decodedChlo.legacy_session_id, expectedChlo.legacy_session_id));
  checkExtensions(decodedChlo.extensions, expectedChlo.extensions);
  EXPECT_EQ(decodedChlo.random, expectedChlo.random);
  EXPECT_EQ(decodedChlo.cipher_suites, expectedChlo.cipher_suites);
  EXPECT_EQ(
      decodedChlo.legacy_compression_methods,
      expectedChlo.legacy_compression_methods);
}

ECHConfig getInvalidVECHConfig() {
  // Add invalid config
  ECHConfig invalidConfig;
  invalidConfig.version = ECHVersion::Draft15;
  auto configContent = getECHConfigContent();
  configContent.key_config.kem_id = hpke::KEMId::secp384r1;
  invalidConfig.ech_config_content = encode(std::move(configContent));

  return invalidConfig;
}

hpke::SetupResult constructSetupResult(
    const SupportedECHConfig& supportedConfig) {
  auto kex = std::make_unique<MockOpenSSLECKeyExchange256>();
  auto privateKey = getPrivateKey(kP256Key);
  kex->setPrivateKey(std::move(privateKey));
  EXPECT_CALL(*kex, generateKeyPair()).Times(1);

  return constructHpkeSetupResult(
      fizz::DefaultFactory(), std::move(kex), supportedConfig);
}

OuterECHClientHello getTestOuterECHClientHelloWithInner(ClientHello chloInner) {
  auto configContent = getECHConfigContent();
  SupportedECHConfig supportedConfig{
      getECHConfig(),
      configContent.key_config.config_id,
      configContent.maximum_name_length,
      HpkeSymmetricCipherSuite{
          hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256}};

  auto setupResult = constructSetupResult(supportedConfig);
  chloInner.legacy_session_id = folly::IOBuf::copyBuffer(testLegacySessionId);

  std::vector<ExtensionType> outerExtensionTypes = {ExtensionType::key_share};
  return encryptClientHello(
      supportedConfig,
      std::move(chloInner),
      getClientHelloOuter(),
      setupResult,
      folly::none,
      outerExtensionTypes);
}

OuterECHClientHello getTestOuterECHClientHello() {
  return getTestOuterECHClientHelloWithInner(TestMessages::clientHello());
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
  EXPECT_EQ(result.value().configId, 0xFB);
  EXPECT_EQ(result.value().maxLen, 100);
}
auto hpkeContext(OuterECHClientHello& clientECH) {
  // Create HPKE setup prefix
  std::string tlsEchPrefix = "tls ech";
  tlsEchPrefix += '\0';
  auto hpkePrefix = folly::IOBuf::copyBuffer(tlsEchPrefix);
  hpkePrefix->prependChain(encode(getECHConfig()));

  auto kex = std::make_unique<MockOpenSSLECKeyExchange256>();
  kex->setPrivateKey(getPrivateKey(kP256Key));

  auto suiteId = hpke::generateHpkeSuiteId(
      NamedGroup::secp256r1,
      HashFunction::Sha256,
      CipherSuite::TLS_AES_128_GCM_SHA256);

  auto dhkem = std::make_unique<DHKEM>(
      std::move(kex),
      NamedGroup::secp256r1,
      std::make_unique<fizz::hpke::Hkdf>(
          fizz::hpke::Hkdf::v1(openssl::hasherFactory<fizz::Sha256>())));

  hpke::SetupParam setupParam{
      std::move(dhkem),
      fizz::DefaultFactory().makeAead(CipherSuite::TLS_AES_128_GCM_SHA256),
      std::make_unique<fizz::hpke::Hkdf>(
          fizz::hpke::Hkdf::v1(openssl::hasherFactory<fizz::Sha256>())),
      std::move(suiteId),
  };

  auto context = setupWithDecap(
      hpke::Mode::Base,
      clientECH.enc->coalesce(),
      folly::none,
      std::move(hpkePrefix),
      folly::none,
      std::move(setupParam));
  return context;
}

} // namespace

TEST(EncryptionTest, TestValidECHConfigContent) {
  // Add config that doesn't work and cannot be supported
  ECHConfigContentDraft invalidConfigContent = getECHConfigContent();
  invalidConfigContent.key_config.kem_id = hpke::KEMId::secp521r1;
  std::vector<ECHConfig> configs;
  ECHConfig invalid;
  invalid.version = ECHVersion::Draft15;
  invalid.ech_config_content = encode(std::move(invalidConfigContent));

  // Add config that works and can be supported
  ECHConfig valid = getECHConfig();

  configs.push_back(std::move(invalid));
  configs.push_back(std::move(valid));

  checkSupportedConfigValid(configs, encode(getECHConfigContent()));
}

TEST(EncryptionTest, TestInvalidECHConfigContent) {
  ECHConfigContentDraft configContent = getECHConfigContent();

  configContent.key_config.kem_id = hpke::KEMId::secp256r1;
  HpkeSymmetricCipherSuite suite{
      hpke::KDFId::Sha512, hpke::AeadId::TLS_AES_128_GCM_SHA256};
  std::vector<HpkeSymmetricCipherSuite> cipher_suites = {suite};
  configContent.key_config.cipher_suites = cipher_suites;

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
  invalid.version = ECHVersion::Draft15;
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
  validConfig.version = ECHVersion::Draft15;
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
  auto clientECH = getTestOuterECHClientHello();
  auto expectedChlo = TestMessages::clientHello();
  // Add a legacy_session_id to match client hello inner used in
  // getTestOuterECHClientHello()
  expectedChlo.legacy_session_id =
      folly::IOBuf::copyBuffer("test legacy session id");
  // expectedChlo should have OuterExtensions
  OuterExtensions expectedOuterExt;
  expectedOuterExt.types = {ExtensionType::key_share};
  auto it = std::find_if(
      expectedChlo.extensions.begin(),
      expectedChlo.extensions.end(),
      [](const auto& ext) {
        return ext.extension_type == ExtensionType::key_share;
      });
  *it = encodeExtension(expectedOuterExt);

  auto context = hpkeContext(clientECH);
  // Get client hello inner by decrypting
  auto clientHelloOuter = getClientHelloOuter();
  auto dummyECH = getTestOuterECHClientHello();
  dummyECH.payload->coalesce();
  memset(dummyECH.payload->writableData(), 0, dummyECH.payload->length());
  clientHelloOuter.extensions.push_back(encodeExtension(dummyECH));

  auto clientHelloOuterAad = encode(clientHelloOuter);

  std::unique_ptr<folly::IOBuf> gotClientHelloInner =
      context->open(clientHelloOuterAad.get(), std::move(clientECH.payload));

  folly::io::Cursor encodedECHInnerCursor(gotClientHelloInner.get());
  auto gotChlo = decode<ClientHello>(encodedECHInnerCursor);

  // Check padding
  auto configContent = getECHConfigContent();
  auto paddingSize = calculateECHPadding(
      gotChlo,
      encodedECHInnerCursor.getCurrentPosition(),
      configContent.maximum_name_length);
  for (size_t i = 0; i < paddingSize; i++) {
    EXPECT_EQ(0, encodedECHInnerCursor.read<uint8_t>());
  }
  EXPECT_TRUE(encodedECHInnerCursor.isAtEnd());

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
  auto testECH = getTestOuterECHClientHello();
  chloOuter.extensions.push_back(encodeExtension(testECH));

  auto kex = std::make_unique<MockOpenSSLECKeyExchange256>();
  auto privateKey = getPrivateKey(kP256Key);
  kex->setPrivateKey(std::move(privateKey));

  auto context = setupDecryptionContext(
      fizz::DefaultFactory(),
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
      ECHVersion::Draft15,
      context);

  checkDecodedChlo(std::move(chlo), std::move(expectedChlo));
}

TEST(EncryptionTest, TestInnerClientHelloOuterExtensionsSuccess) {
  auto innerChlo = TestMessages::clientHello();

  // Extract key_share and replace with OuterExtensions
  Extension encodedClientKeyShare;
  OuterExtensions outer;
  outer.types = {ExtensionType::key_share};
  auto it = std::find_if(
      innerChlo.extensions.begin(), innerChlo.extensions.end(), [](auto& ext) {
        return ext.extension_type == ExtensionType::key_share;
      });
  if (it != innerChlo.extensions.end()) {
    encodedClientKeyShare = it->clone();
    innerChlo.extensions.erase(it);
  }
  innerChlo.extensions.push_back(encodeExtension(std::move(outer)));
  auto clientECH = getTestOuterECHClientHelloWithInner(std::move(innerChlo));
  auto context = hpkeContext(clientECH);
  auto outerChlo = getClientHelloOuter();
  outerChlo.extensions.push_back(encodeExtension(clientECH));

  auto decryptedChlo = decryptECHWithContext(
      outerChlo,
      getECHConfig(),
      clientECH.cipher_suite,
      std::move(clientECH.enc),
      std::move(clientECH.config_id),
      std::move(clientECH.payload),
      ECHVersion::Draft15,
      context);

  auto decryptedClientKeyShare =
      findExtension(decryptedChlo.extensions, ExtensionType::key_share)
          ->clone();
  EXPECT_EQ(
      decryptedClientKeyShare.extension_type,
      encodedClientKeyShare.extension_type);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      decryptedClientKeyShare.extension_data,
      encodedClientKeyShare.extension_data));
}

TEST(EncryptionTest, TestInnerClientHelloOuterExtensionsContainsECH) {
  auto innerChlo = TestMessages::clientHello();
  // Add OuterExtensions with ECH extension. Should blow up on decrypt.
  OuterExtensions outer;
  outer.types = {ExtensionType::encrypted_client_hello};
  innerChlo.extensions.push_back(encodeExtension(std::move(outer)));
  auto clientECH = getTestOuterECHClientHelloWithInner(std::move(innerChlo));
  auto context = hpkeContext(clientECH);
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
          ECHVersion::Draft15,
          context),
      OuterExtensionsError);
}

TEST(EncryptionTest, TestInnerClientHelloOuterExtensionsContainsDupes) {
  auto innerChlo = TestMessages::clientHello();
  // Add OuterExtensions with SNI extension. Should blow up on decrypt.
  OuterExtensions outer;
  outer.types = {ExtensionType::server_name};
  innerChlo.extensions.push_back(encodeExtension(std::move(outer)));
  auto clientECH = getTestOuterECHClientHelloWithInner(std::move(innerChlo));

  auto context = hpkeContext(clientECH);
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
          ECHVersion::Draft15,
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

TEST(EncryptionTest, TestMakeDummyServerHello) {
  auto shlo = TestMessages::serverHello();
  auto dummyShlo = makeDummyServerHello(shlo);
  EXPECT_EQ(shlo.legacy_version, dummyShlo.legacy_version);
  // Check that the non-ECH part of the random is preserved.
  auto randomUnmodifiedRange = folly::Range(
      shlo.random.begin(), shlo.random.end() - kEchAcceptConfirmationSize);
  auto dummyRandomUnmodifiedRange = folly::Range(
      dummyShlo.random.begin(),
      dummyShlo.random.end() - kEchAcceptConfirmationSize);
  EXPECT_EQ(randomUnmodifiedRange, dummyRandomUnmodifiedRange);

  // Check that the end is zeroes.
  for (auto it = dummyShlo.random.end() - kEchAcceptConfirmationSize;
       it != dummyShlo.random.end();
       it++) {
    EXPECT_EQ(*it, 0);
  }

  EXPECT_TRUE(folly::IOBufEqualTo()(
      shlo.legacy_session_id_echo, dummyShlo.legacy_session_id_echo));
  EXPECT_EQ(shlo.cipher_suite, dummyShlo.cipher_suite);
  EXPECT_EQ(
      shlo.legacy_compression_method, dummyShlo.legacy_compression_method);
  EXPECT_EQ(shlo.extensions.size(), dummyShlo.extensions.size());
  for (size_t i = 0; i < shlo.extensions.size(); i++) {
    EXPECT_EQ(
        shlo.extensions[i].extension_type,
        dummyShlo.extensions[i].extension_type);
    EXPECT_TRUE(folly::IOBufEqualTo()(
        shlo.extensions[i].extension_data,
        dummyShlo.extensions[i].extension_data));
  }
}

namespace {

Extension makeDummyECHExtension() {
  auto buf = folly::IOBuf::create(kEchAcceptConfirmationSize);
  memset(buf->writableData(), 0xAC, kEchAcceptConfirmationSize);
  buf->append(kEchAcceptConfirmationSize);
  return {ExtensionType::encrypted_client_hello, std::move(buf)};
}

} // namespace

TEST(EncryptionTest, TestMakeDummyHRR) {
  auto hrr = TestMessages::helloRetryRequest();
  hrr.extensions.push_back(makeDummyECHExtension());
  auto dummyHrr = makeDummyHRR(hrr);
  EXPECT_EQ(hrr.legacy_version, dummyHrr.legacy_version);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      hrr.legacy_session_id_echo, dummyHrr.legacy_session_id_echo));
  EXPECT_EQ(hrr.cipher_suite, dummyHrr.cipher_suite);
  EXPECT_EQ(hrr.legacy_compression_method, dummyHrr.legacy_compression_method);
  EXPECT_EQ(hrr.extensions.size(), dummyHrr.extensions.size());
  for (size_t i = 0; i < hrr.extensions.size(); i++) {
    EXPECT_EQ(
        hrr.extensions[i].extension_type,
        dummyHrr.extensions[i].extension_type);
    // If extension type is ech, expect it to be zeroed.
    if (hrr.extensions[i].extension_type ==
        ExtensionType::encrypted_client_hello) {
      EXPECT_EQ(
          dummyHrr.extensions[i].extension_data->computeChainDataLength(),
          hrr.extensions[i].extension_data->computeChainDataLength());
      auto dummyECH = dummyHrr.extensions[i].extension_data->coalesce();
      EXPECT_TRUE(
          std::all_of(dummyECH.begin(), dummyECH.end(), [](const auto& c) {
            return c == 0;
          }));
    } else {
      EXPECT_TRUE(folly::IOBufEqualTo()(
          hrr.extensions[i].extension_data,
          dummyHrr.extensions[i].extension_data));
    }
  }
}

MATCHER_P(DummyShloMatch, shlo, "") {
  return folly::IOBufEqualTo()(
      encodeHandshake(makeDummyServerHello(*shlo)), arg);
}

TEST(EncryptionTest, TestSetShloAcceptance) {
  auto shlo = TestMessages::serverHello();
  auto context = std::make_unique<MockHandshakeContext>();
  auto scheduler = std::make_unique<MockKeyScheduler>();
  auto acceptSecret = std::vector<uint8_t>(kEchAcceptConfirmationSize, 0xAC);
  EXPECT_CALL(*context, appendToTranscript(DummyShloMatch(&shlo)));
  EXPECT_CALL(*context, getHandshakeContext()).WillOnce(Invoke([]() {
    return folly::IOBuf::copyBuffer("dummyshlo");
  }));
  EXPECT_CALL(
      *scheduler,
      getSecret(EarlySecrets::ECHAcceptConfirmation, RangeMatches("dummyshlo")))
      .WillOnce(InvokeWithoutArgs([&]() {
        return DerivedSecret(acceptSecret, EarlySecrets::ECHAcceptConfirmation);
      }));
  std::unique_ptr<KeyScheduler> schedulerBasePtr = std::move(scheduler);
  setAcceptConfirmation(shlo, std::move(context), std::move(schedulerBasePtr));
  auto shloIt = shlo.random.end() - kEchAcceptConfirmationSize;
  auto secretIt = acceptSecret.begin();
  for (size_t i = 0; i < kEchAcceptConfirmationSize; i++) {
    EXPECT_EQ(*shloIt, *secretIt);
  }
}

MATCHER_P(DummyHrrMatch, hrr, "") {
  return folly::IOBufEqualTo()(encodeHandshake(makeDummyHRR(*hrr)), arg);
}

TEST(EncryptionTest, TestSetHRRAcceptance) {
  auto hrr = TestMessages::helloRetryRequest();
  auto context = std::make_unique<MockHandshakeContext>();
  auto scheduler = std::make_unique<MockKeyScheduler>();
  auto acceptSecret = std::vector<uint8_t>(kEchAcceptConfirmationSize, 0xAC);
  EXPECT_CALL(*context, appendToTranscript(DummyHrrMatch(&hrr)));
  EXPECT_CALL(*context, getHandshakeContext()).WillOnce(Invoke([]() {
    return folly::IOBuf::copyBuffer("dummyhrr");
  }));
  EXPECT_CALL(
      *scheduler,
      getSecret(
          EarlySecrets::HRRECHAcceptConfirmation, RangeMatches("dummyhrr")))
      .WillOnce(InvokeWithoutArgs([&]() {
        return DerivedSecret(
            acceptSecret, EarlySecrets::HRRECHAcceptConfirmation);
      }));
  std::unique_ptr<KeyScheduler> schedulerBasePtr = std::move(scheduler);
  setAcceptConfirmation(hrr, std::move(context), std::move(schedulerBasePtr));
  auto echExtensionRange = hrr.extensions.back().extension_data->coalesce();
  auto echExtensionIt = echExtensionRange.begin();
  auto secretIt = acceptSecret.begin();
  for (size_t i = 0; i < kEchAcceptConfirmationSize; i++) {
    EXPECT_EQ(*echExtensionIt, *secretIt);
  }
}

TEST(EncryptionTest, TestCheckShloAcceptance) {
  auto shlo = TestMessages::serverHello();
  auto context = std::make_unique<MockHandshakeContext>();
  auto scheduler = std::make_unique<MockKeyScheduler>();
  auto acceptSecret = std::vector<uint8_t>(kEchAcceptConfirmationSize, 0xAC);
  std::fill(
      shlo.random.end() - kEchAcceptConfirmationSize, shlo.random.end(), 0xAC);
  EXPECT_CALL(*context, appendToTranscript(DummyShloMatch(&shlo)));
  EXPECT_CALL(*context, getHandshakeContext()).WillOnce(Invoke([]() {
    return folly::IOBuf::copyBuffer("dummyshlo");
  }));
  EXPECT_CALL(
      *scheduler,
      getSecret(EarlySecrets::ECHAcceptConfirmation, RangeMatches("dummyshlo")))
      .WillOnce(InvokeWithoutArgs([&]() {
        return DerivedSecret(acceptSecret, EarlySecrets::ECHAcceptConfirmation);
      }));
  std::unique_ptr<KeyScheduler> schedulerBasePtr = std::move(scheduler);
  EXPECT_TRUE(
      checkECHAccepted(shlo, std::move(context), std::move(schedulerBasePtr)));
}

TEST(EncryptionTest, TestCheckHrrAcceptance) {
  auto hrr = TestMessages::helloRetryRequest();
  hrr.extensions.push_back(makeDummyECHExtension());
  auto context = std::make_unique<MockHandshakeContext>();
  auto scheduler = std::make_unique<MockKeyScheduler>();
  auto acceptSecret = std::vector<uint8_t>(kEchAcceptConfirmationSize, 0xAC);
  EXPECT_CALL(*context, appendToTranscript(DummyHrrMatch(&hrr)));
  EXPECT_CALL(*context, getHandshakeContext()).WillOnce(Invoke([]() {
    return folly::IOBuf::copyBuffer("dummyhrr");
  }));
  EXPECT_CALL(
      *scheduler,
      getSecret(
          EarlySecrets::HRRECHAcceptConfirmation, RangeMatches("dummyhrr")))
      .WillOnce(InvokeWithoutArgs([&]() {
        return DerivedSecret(
            acceptSecret, EarlySecrets::HRRECHAcceptConfirmation);
      }));
  std::unique_ptr<KeyScheduler> schedulerBasePtr = std::move(scheduler);
  EXPECT_TRUE(
      checkECHAccepted(hrr, std::move(context), std::move(schedulerBasePtr)));
}

TEST(EncryptionTest, TestGenerateGreasePsk) {
  MockFactory factory;
  factory.setDefaults();

  // If no psk extension is present, expect no GREASE PSK.
  EXPECT_EQ(
      generateGreasePSK(TestMessages::clientHello(), &factory), folly::none);

  auto chlo = TestMessages::clientHelloPsk();
  auto psk = getExtension<ClientPresharedKey>(chlo.extensions);

  // Check that data is replaced with random data for GREASE PSK
  auto greasePsk = generateGreasePSK(chlo, &factory);
  EXPECT_TRUE(greasePsk.has_value());
  EXPECT_EQ(greasePsk->identities.size(), psk->identities.size());
  EXPECT_EQ(greasePsk->binders.size(), psk->binders.size());
  for (size_t i = 0; i < greasePsk->identities.size(); i++) {
    auto binderSz = psk->binders[i].binder->computeChainDataLength();
    // We set fixed size (16 bytes) identity for grease PSK
    auto randomId = folly::IOBuf::copyBuffer(std::string(16, 0x44));
    auto randomBinder = folly::IOBuf::copyBuffer(std::string(binderSz, 0x44));
    EXPECT_TRUE(
        folly::IOBufEqualTo()(greasePsk->identities[i].psk_identity, randomId));
    EXPECT_TRUE(
        folly::IOBufEqualTo()(greasePsk->binders[i].binder, randomBinder));
  }

  // Check HRR GREASE. Same as above, but preserves identities.
  auto hrrGreasePsk = generateGreasePSKForHRR(*psk, &factory);
  EXPECT_EQ(hrrGreasePsk.identities.size(), psk->identities.size());
  EXPECT_EQ(hrrGreasePsk.binders.size(), psk->binders.size());
  for (size_t i = 0; i < hrrGreasePsk.identities.size(); i++) {
    auto binderSz = psk->binders[i].binder->computeChainDataLength();
    auto randomBinder = folly::IOBuf::copyBuffer(std::string(binderSz, 0x44));
    EXPECT_TRUE(folly::IOBufEqualTo()(
        hrrGreasePsk.identities[i].psk_identity,
        psk->identities[i].psk_identity));
    EXPECT_TRUE(
        folly::IOBufEqualTo()(hrrGreasePsk.binders[i].binder, randomBinder));
  }
}

TEST(EncryptionTest, TestGenerateAndReplaceOuterExtensions) {
  // Make some arbitrary extensions
  Extension supportedVersions = encodeExtension(SupportedVersions());
  Extension supportedGroups = encodeExtension(SupportedGroups());
  Extension keyShare = encodeExtension(ClientKeyShare());
  Extension signatureAlgorithms = encodeExtension(SignatureAlgorithms());
  Extension certificateAuthorities = encodeExtension(CertificateAuthorities());

  std::vector<Extension> exts;
  exts.push_back(supportedVersions.clone());
  exts.push_back(supportedGroups.clone());
  exts.push_back(keyShare.clone());
  exts.push_back(signatureAlgorithms.clone());
  exts.push_back(certificateAuthorities.clone());

  static const std::vector<ExtensionType> outerExtensionTypes = {
      ExtensionType::signature_algorithms, ExtensionType::supported_groups};
  OuterExtensions expectedOuterExt;
  // Types in the constructed OuterExtensions should be in the same order as in
  // the original extensions list
  expectedOuterExt.types = {
      ExtensionType::supported_groups, ExtensionType::signature_algorithms};
  std::vector<Extension> expectedExts;
  expectedExts.push_back(supportedVersions.clone());
  expectedExts.push_back(encodeExtension(expectedOuterExt));
  expectedExts.push_back(keyShare.clone());
  expectedExts.push_back(certificateAuthorities.clone());

  auto extsWithOuterExtensions =
      generateAndReplaceOuterExtensions(std::move(exts), outerExtensionTypes);
  checkExtensions(extsWithOuterExtensions, expectedExts);
}

} // namespace test
} // namespace ech
} // namespace fizz
