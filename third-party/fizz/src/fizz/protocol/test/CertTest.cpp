/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/CertUtils.h>
#include <fizz/protocol/OpenSSLPeerCertImpl.h>
#include <fizz/protocol/OpenSSLSelfCertImpl.h>
#include <folly/String.h>

using namespace folly;
using namespace testing;

namespace fizz {
namespace test {
struct RSATest;

struct P256Test {
  static constexpr KeyType Type = KeyType::P256;
  static constexpr SignatureScheme Scheme =
      SignatureScheme::ecdsa_secp256r1_sha256;

  using Invalid = RSATest;
};

struct P384Test {
  static constexpr KeyType Type = KeyType::P384;
  static constexpr SignatureScheme Scheme =
      SignatureScheme::ecdsa_secp384r1_sha384;

  using Invalid = RSATest;
};

struct P521Test {
  static constexpr KeyType Type = KeyType::P521;
  static constexpr SignatureScheme Scheme =
      SignatureScheme::ecdsa_secp521r1_sha512;

  using Invalid = RSATest;
};

struct RSATest {
  static constexpr KeyType Type = KeyType::RSA;
  static constexpr SignatureScheme Scheme = SignatureScheme::rsa_pss_sha256;

  using Invalid = P256Test;
};

struct Ed25519Test {
  static constexpr KeyType Type = KeyType::ED25519;
  static constexpr SignatureScheme Scheme = SignatureScheme::ed25519;

  using Invalid = P256Test;
};

template <typename T>
static ssl::X509UniquePtr getCert();

template <>
ssl::X509UniquePtr getCert<P256Test>() {
  return fizz::test::getCert(kP256Certificate);
}

template <>
ssl::X509UniquePtr getCert<P384Test>() {
  return fizz::test::getCert(kP384Certificate);
}

template <>
ssl::X509UniquePtr getCert<P521Test>() {
  return fizz::test::getCert(kP521Certificate);
}

template <>
ssl::X509UniquePtr getCert<RSATest>() {
  return fizz::test::getCert(kRSACertificate);
}

#if FIZZ_OPENSSL_HAS_ED25519
template <>
ssl::X509UniquePtr getCert<Ed25519Test>() {
  return fizz::test::getCert(kEd25519Certificate);
}
#endif

template <typename T>
static ssl::EvpPkeyUniquePtr getKey();

template <>
ssl::EvpPkeyUniquePtr getKey<P256Test>() {
  return getPrivateKey(kP256Key);
}

template <>
ssl::EvpPkeyUniquePtr getKey<P384Test>() {
  return getPrivateKey(kP384Key);
}

template <>
ssl::EvpPkeyUniquePtr getKey<P521Test>() {
  return getPrivateKey(kP521Key);
}

template <>
ssl::EvpPkeyUniquePtr getKey<RSATest>() {
  return getPrivateKey(kRSAKey);
}

#if FIZZ_OPENSSL_HAS_ED25519
template <>
ssl::EvpPkeyUniquePtr getKey<Ed25519Test>() {
  return getPrivateKey(kEd25519Key);
}
#endif

template <typename T>
class CertTestTyped : public Test {
 public:
  void SetUp() override {
    OpenSSL_add_all_algorithms();
  }
};

using KeyTypes = Types<
#if FIZZ_OPENSSL_HAS_ED25519
    Ed25519Test,
#endif
    P256Test,
    P384Test,
    P521Test,
    RSATest>;
TYPED_TEST_SUITE(CertTestTyped, KeyTypes);

TEST(CertTest, GetIdentity) {
  auto cert = getCert(kP256Certificate);
  auto key = getPrivateKey(kP256Key);
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(std::move(cert));
  OpenSSLSelfCertImpl<KeyType::P256> certificate(
      std::move(key), std::move(certs));
  EXPECT_EQ(certificate.getIdentity(), "Fizz");
  EXPECT_EQ(certificate.getAltIdentities().size(), 0);
}

TEST(CertTest, GetAltIdentity) {
  auto cert = getCert(kRSACertificate);
  auto key = getPrivateKey(kRSAKey);
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(std::move(cert));
  OpenSSLSelfCertImpl<KeyType::RSA> certificate(
      std::move(key), std::move(certs));
  EXPECT_EQ(certificate.getIdentity(), "Fizz");
  auto alts = certificate.getAltIdentities();
  EXPECT_EQ(alts.size(), 3);
  EXPECT_EQ(alts[0], "*.fizz.com");
  EXPECT_EQ(alts[1], "fizz.com");
  EXPECT_EQ(alts[2], "example.net");
}

TEST(CertTest, GetCertMessage) {
  auto cert = getCert(kP256Certificate);
  auto key = getPrivateKey(kP256Key);
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(std::move(cert));
  OpenSSLSelfCertImpl<KeyType::P256> certificate(
      std::move(key), std::move(certs));
  auto msg = certificate.getCertMessage();
  ASSERT_EQ(msg.certificate_list.size(), 1);
  auto& firstCertEntry = msg.certificate_list[0];
  auto firstCertData = firstCertEntry.cert_data->coalesce();
  auto firstCertDataPtr = firstCertData.data();
  folly::ssl::X509UniquePtr firstEncodedCert(
      d2i_X509(nullptr, &firstCertDataPtr, firstCertData.size()));
  CHECK(firstEncodedCert);

  auto certCopy = getCert(kP256Certificate);
  EXPECT_EQ(X509_cmp(firstEncodedCert.get(), certCopy.get()), 0);
}

// example taken from https://tlswg.github.io/tls13-spec/#certificate-verify
TEST(CertTest, PrepareSignData) {
  std::array<uint8_t, 32> toBeSigned;
  memset(toBeSigned.data(), 1, toBeSigned.size());
  auto out = CertUtils::prepareSignData(
      CertificateVerifyContext::Server, folly::range(toBeSigned));
  auto hex = hexlify(out->moveToFbString());
  std::string expected =
      "2020202020202020202020202020202020202020202020202020202020202020"
      "2020202020202020202020202020202020202020202020202020202020202020"
      "544c5320312e332c207365727665722043657274696669636174655665726966"
      "79"
      "00"
      "0101010101010101010101010101010101010101010101010101010101010101";
  EXPECT_EQ(hex, expected);
}

TEST(CertTest, MakePeerCertEmpty) {
  EXPECT_THROW(
      CertUtils::makePeerCert(IOBuf::copyBuffer("")), std::runtime_error);
}

TEST(CertTest, MakePeerCertJunk) {
  EXPECT_THROW(
      CertUtils::makePeerCert(IOBuf::copyBuffer("blah")), std::runtime_error);
}

TEST(CertTest, PeerCertGetX509) {
  OpenSSLPeerCertImpl<KeyType::P256> peerCert(getCert(kP256Certificate));
  auto x509 = peerCert.getX509();
  EXPECT_NE(x509.get(), nullptr);
}

TEST(CertTest, GetIdentityLogic) {
  auto selfCert = CertUtils::makeSelfCert(
      kCertWithNoCNButWithSANs.str(), kCertWithNoCNButWithSANsKey.str(), "");
  EXPECT_EQ("O = interop runner", selfCert->getIdentity());

  auto peerCert =
      CertUtils::makePeerCert(fizz::test::getCert(kCertWithNoCNButWithSANs));
  EXPECT_EQ("O = interop runner", peerCert->getIdentity());
}

TYPED_TEST(CertTestTyped, MatchingCert) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  OpenSSLSelfCertImpl<TypeParam::Type> certificate(
      getKey<TypeParam>(), std::move(certs));
}

TYPED_TEST(CertTestTyped, MismatchedCert) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  EXPECT_THROW(
      OpenSSLSelfCertImpl<TypeParam::Type> certificate(
          getKey<typename TypeParam::Invalid>(), std::move(certs)),
      std::runtime_error);
}

TYPED_TEST(CertTestTyped, SigSchemes) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  OpenSSLSelfCertImpl<TypeParam::Type> certificate(
      getKey<TypeParam>(), std::move(certs));

  std::vector<SignatureScheme> expected{TypeParam::Scheme};
  EXPECT_EQ(certificate.getSigSchemes(), expected);
}

TYPED_TEST(CertTestTyped, TestSignVerify) {
  OpenSSLPeerCertImpl<TypeParam::Type> peerCert(getCert<TypeParam>());
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  OpenSSLSelfCertImpl<TypeParam::Type> selfCert(
      getKey<TypeParam>(), std::move(certs));

  StringPiece tbs{"ToBeSigned"};
  auto sig =
      selfCert.sign(TypeParam::Scheme, CertificateVerifyContext::Server, tbs);
  peerCert.verify(
      TypeParam::Scheme,
      CertificateVerifyContext::Server,
      tbs,
      sig->coalesce());
}

TYPED_TEST(CertTestTyped, TestSignVerifyBitFlip) {
  OpenSSLPeerCertImpl<TypeParam::Type> peerCert(getCert<TypeParam>());
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  OpenSSLSelfCertImpl<TypeParam::Type> selfCert(
      getKey<TypeParam>(), std::move(certs));

  StringPiece tbs{"ToBeSigned"};
  auto sig =
      selfCert.sign(TypeParam::Scheme, CertificateVerifyContext::Server, tbs);
  sig->writableData()[1] ^= 0x20;
  EXPECT_THROW(
      peerCert.verify(
          TypeParam::Scheme,
          CertificateVerifyContext::Server,
          tbs,
          sig->coalesce()),
      std::runtime_error);
}

TYPED_TEST(CertTestTyped, TestSignVerifyWrongSize) {
  OpenSSLPeerCertImpl<TypeParam::Type> peerCert(getCert<TypeParam>());
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  OpenSSLSelfCertImpl<TypeParam::Type> selfCert(
      getKey<TypeParam>(), std::move(certs));

  StringPiece tbs{"ToBeSigned"};
  auto sig =
      selfCert.sign(TypeParam::Scheme, CertificateVerifyContext::Server, tbs);
  sig->prependChain(IOBuf::copyBuffer("x"));
  EXPECT_THROW(
      peerCert.verify(
          TypeParam::Scheme,
          CertificateVerifyContext::Server,
          tbs,
          sig->coalesce()),
      std::runtime_error);
}

TYPED_TEST(CertTestTyped, TestVerifyWrongScheme) {
  OpenSSLPeerCertImpl<TypeParam::Type> peerCert(getCert<TypeParam>());
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  OpenSSLSelfCertImpl<TypeParam::Type> selfCert(
      getKey<TypeParam>(), std::move(certs));

  StringPiece tbs{"ToBeSigned"};
  auto sig =
      selfCert.sign(TypeParam::Scheme, CertificateVerifyContext::Server, tbs);
  EXPECT_THROW(
      peerCert.verify(
          TypeParam::Invalid::Scheme,
          CertificateVerifyContext::Server,
          tbs,
          sig->coalesce()),
      std::runtime_error);
}

TYPED_TEST(CertTestTyped, TestVerifyDecodedCert) {
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(getCert<TypeParam>());
  auto msg = CertUtils::getCertMessage(certs, nullptr);
  OpenSSLSelfCertImpl<TypeParam::Type> selfCert(
      getKey<TypeParam>(), std::move(certs));

  auto peerCert = CertUtils::makePeerCert(
      std::move(msg.certificate_list.front().cert_data));

  StringPiece tbs{"ToBeSigned"};
  auto sig =
      selfCert.sign(TypeParam::Scheme, CertificateVerifyContext::Server, tbs);
  peerCert->verify(
      TypeParam::Scheme,
      CertificateVerifyContext::Server,
      tbs,
      sig->coalesce());
}
} // namespace test
} // namespace fizz
