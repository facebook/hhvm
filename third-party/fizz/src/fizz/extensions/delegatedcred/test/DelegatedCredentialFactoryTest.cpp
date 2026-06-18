/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/backend/openssl/certificate/OpenSSLPeerCertImpl.h>
#include <fizz/crypto/Utils.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialFactory.h>
#include <fizz/extensions/delegatedcred/PeerDelegatedCredential.h>
#include <fizz/extensions/delegatedcred/test/TestData.h>

using namespace folly;

using namespace testing;
using namespace fizz::test;

namespace fizz {
namespace extensions {
namespace test {

class DelegatedCredentialFactoryTest : public Test {
 public:
  void SetUp() override {
    Error err;
    FIZZ_THROW_ON_ERROR(CryptoUtils::init(err), err);
  }

  CertificateEntry generateEntry() const {
    CertificateEntry entry;
    entry.cert_data = getCertData(kDelegationUsageCert);

    Extension ext;
    ext.extension_type = ExtensionType::delegated_credential;
    auto data = unhexlify(kDelegatedCred);
    ext.extension_data = folly::IOBuf::copyBuffer(data.data(), data.size());
    entry.extensions.push_back(std::move(ext));
    return entry;
  }

  void expectThrows(std::function<void()> f, std::string errorStr) {
    std::string what;
    try {
      f();
    } catch (const FizzException& e) {
      what = e.what();
    }

    EXPECT_THAT(what, HasSubstr(errorStr));
  }

  DelegatedCredentialFactory factory_;
};

TEST_F(DelegatedCredentialFactoryTest, TestCredentialParsing) {
  auto entry = generateEntry();
  std::unique_ptr<PeerCert> cert;
  Error err;
  EXPECT_EQ(
      factory_.makePeerCert(cert, err, std::move(entry), true),
      Status::Success);
  EXPECT_TRUE(cert);
  EXPECT_TRUE(
      typeid(*cert) ==
      typeid(PeerDelegatedCredentialImpl<openssl::KeyType::P256>));
}

TEST_F(DelegatedCredentialFactoryTest, TestCredentialParsingNonLeaf) {
  auto entry = generateEntry();
  std::unique_ptr<PeerCert> cert;
  Error err;
  EXPECT_EQ(
      factory_.makePeerCert(cert, err, std::move(entry), false),
      Status::Success);
  EXPECT_TRUE(cert);
  EXPECT_TRUE(
      typeid(*cert) ==
      typeid(openssl::OpenSSLPeerCertImpl<openssl::KeyType::P256>));
}

TEST_F(DelegatedCredentialFactoryTest, TestCredentialNoCertEntryExtension) {
  auto entry = generateEntry();
  entry.extensions.clear();
  std::unique_ptr<PeerCert> cert;
  Error err;
  EXPECT_EQ(
      factory_.makePeerCert(cert, err, std::move(entry), true),
      Status::Success);
  EXPECT_TRUE(cert);
  EXPECT_TRUE(
      typeid(*cert) ==
      typeid(openssl::OpenSSLPeerCertImpl<openssl::KeyType::P256>));
}
} // namespace test
} // namespace extensions
} // namespace fizz
