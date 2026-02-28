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

using namespace folly;

using namespace testing;
using namespace fizz::test;

namespace fizz {
namespace extensions {
namespace test {

StringPiece kDelegatedCred{
    "001ce0d1040300005b3059301306072a8648ce3d020106082a8648ce3d030107"
    "03420004db94b7b305323633ccc5a5f12a3b07c22bbf86e5d531ed94d09c5bfe"
    "860b72e5dc73b8267729f34150a6422cbdc87484a535125ff3a02a03372c0969"
    "3abf050504030048304602210089a87271db53ca89f4eccbc37e616df92f4b35"
    "1f5774c56da74bacfc2774e434022100af1d4561dab905345344475a33c8f132"
    "0d82978beae2fca0f34b8d40713b92ac"};

StringPiece kCert = R"(
-----BEGIN CERTIFICATE-----
MIIB6TCCAY+gAwIBAgIJAKlQpSahHUIWMAoGCCqGSM49BAMCMEIxCzAJBgNVBAYT
AlhYMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t
cGFueSBMdGQwHhcNMTkwNTI0MTk1MjU3WhcNMjAwNTIzMTk1MjU3WjBCMQswCQYD
VQQGEwJYWDEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0
IENvbXBhbnkgTHRkMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE8RC+4O48vtBh
JhSn3/wuzfygo/AQGGNMavAb5YnZpy6rMaY9UG3OlFfkRRmvETlbn3CXD0klXuc/
wYCKoVXGYqNuMGwwHQYDVR0OBBYEFC892QWimVBjX1AODjjL+SqTN1meMB8GA1Ud
IwQYMBaAFC892QWimVBjX1AODjjL+SqTN1meMAwGA1UdEwQFMAMBAf8wCwYDVR0P
BAQDAgHmMA8GCSsGAQQBgtpLLAQCBQAwCgYIKoZIzj0EAwIDSAAwRQIhAPoWbJWf
Fw+uQ6c27kul/uTNIF4GOEUCmCWVvc6qkhHVAiBKTBrUi8h8g/U0yQ4prS0/wfkw
FghrPnYCODq235mY2A==
-----END CERTIFICATE-----
)";

class DelegatedCredentialFactoryTest : public Test {
 public:
  void SetUp() override {
    CryptoUtils::init();
  }

  CertificateEntry generateEntry() const {
    CertificateEntry entry;
    entry.cert_data = getCertData(kCert);

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
  auto cert = factory_.makePeerCert(std::move(entry), true);
  EXPECT_TRUE(cert);
  EXPECT_TRUE(
      typeid(*cert) ==
      typeid(PeerDelegatedCredentialImpl<openssl::KeyType::P256>));
}

TEST_F(DelegatedCredentialFactoryTest, TestCredentialParsingNonLeaf) {
  auto entry = generateEntry();
  auto cert = factory_.makePeerCert(std::move(entry), false);
  EXPECT_TRUE(cert);
  EXPECT_TRUE(
      typeid(*cert) ==
      typeid(openssl::OpenSSLPeerCertImpl<openssl::KeyType::P256>));
}

TEST_F(DelegatedCredentialFactoryTest, TestCredentialNoCertEntryExtension) {
  auto entry = generateEntry();
  entry.extensions.clear();
  auto cert = factory_.makePeerCert(std::move(entry), true);
  EXPECT_TRUE(cert);
  EXPECT_TRUE(
      typeid(*cert) ==
      typeid(openssl::OpenSSLPeerCertImpl<openssl::KeyType::P256>));
}
} // namespace test
} // namespace extensions
} // namespace fizz
