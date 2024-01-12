/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/crypto/Utils.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialFactory.h>
#include <fizz/extensions/delegatedcred/PeerDelegatedCredential.h>
#include <fizz/protocol/OpenSSLPeerCertImpl.h>
#include <fizz/protocol/clock/test/Mocks.h>

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

StringPiece kCertNoDelegatedExtension = R"(
-----BEGIN CERTIFICATE-----
MIIB2DCCAX6gAwIBAgIJAIEdGlbc/R/BMAoGCCqGSM49BAMCMEIxCzAJBgNVBAYT
AlhYMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t
cGFueSBMdGQwHhcNMTkwNjEwMjExNjEwWhcNMjAwNjA5MjExNjEwWjBCMQswCQYD
VQQGEwJYWDEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0
IENvbXBhbnkgTHRkMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE7cnufRVxTjuK
xcBV4IQ35xkjIIICtm05sLlsriVbIcMj55kAVZHkEHnezgEIq6io8BRy+4m8gXYs
X52uCjXmMKNdMFswHQYDVR0OBBYEFBXp/0ABAVMfEpQFDi0K8MK5HEUsMB8GA1Ud
IwQYMBaAFBXp/0ABAVMfEpQFDi0K8MK5HEUsMAwGA1UdEwQFMAMBAf8wCwYDVR0P
BAQDAgHmMAoGCCqGSM49BAMCA0gAMEUCIQDV1EqSNhLD+v6XhSGmoCxw6mnuHv2p
wLj4M2gxgs6VmQIgBLB8W3th4WlHE7+C6YPeX6n834ceZ5dBi6FQLYKgpXY=
-----END CERTIFICATE-----
)";

StringPiece kCertNoKeyUsage = R"(
-----BEGIN CERTIFICATE-----
MIIB3DCCAYKgAwIBAgIJAIueLstLYzuHMAoGCCqGSM49BAMCMEIxCzAJBgNVBAYT
AlhYMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t
cGFueSBMdGQwHhcNMTkwNjEwMjExNjM1WhcNMjAwNjA5MjExNjM1WjBCMQswCQYD
VQQGEwJYWDEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0
IENvbXBhbnkgTHRkMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEOiX1bVPJgpse
oV8UxhOZ5kBynFLu0Iazp8axzziiILdW3j9dG7z2sCBaPiJntn7whfEOwD41mcJq
UNssOm18IKNhMF8wHQYDVR0OBBYEFHdcgbW5+8piYNQdNy2dnXGuPvBQMB8GA1Ud
IwQYMBaAFHdcgbW5+8piYNQdNy2dnXGuPvBQMAwGA1UdEwQFMAMBAf8wDwYJKwYB
BAGC2kssBAIFADAKBggqhkjOPQQDAgNIADBFAiEAl01tUrb6x6E/SsRuPOKteKHZ
qf+khcoJYtl3FQiBNrECIHP6Qxr3ZXysyHi0uGfkGqPVVLN9Knl7DXVo6CYVQKKl
-----END CERTIFICATE-----
)";

StringPiece kCertWrongKeyUsage = R"(
-----BEGIN CERTIFICATE-----
MIIB6TCCAY+gAwIBAgIJAMfSoojFcEPAMAoGCCqGSM49BAMCMEIxCzAJBgNVBAYT
AlhYMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t
cGFueSBMdGQwHhcNMTkwNjEwMjEzMDQwWhcNMjAwNjA5MjEzMDQwWjBCMQswCQYD
VQQGEwJYWDEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0
IENvbXBhbnkgTHRkMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEgyT1AYOI+lLL
SGKWkPAIXOkQtYXMdhz1+YmvlD4O/tUpedv9UzKb+6YQ/pZj0Kg+IzilwIJSL+aT
SAG8fLAzS6NuMGwwHQYDVR0OBBYEFHB8ZwSEBZitRX6GbOxW0+ProEX4MB8GA1Ud
IwQYMBaAFHB8ZwSEBZitRX6GbOxW0+ProEX4MAwGA1UdEwQFMAMBAf8wCwYDVR0P
BAQDAgFmMA8GCSsGAQQBgtpLLAQCBQAwCgYIKoZIzj0EAwIDSAAwRQIgUMEK+LqG
x+n8WLnzTOj0bIQOfyerUEIRnEiDzwZtM2ACIQDyDF4Gibr14YHUs8ZbP3l37A9c
wNMYE5qiSvpfGd77lw==
-----END CERTIFICATE-----
)";

class DelegatedCredentialFactoryTest : public Test {
 public:
  void SetUp() override {
    CryptoUtils::init();
    clock_ = std::make_shared<MockClock>();
    factory_.setClock(clock_);
    // Default return time to June 10, 2019 6:08:58 PM
    ON_CALL(*clock_, getCurrentTime())
        .WillByDefault(Return(std::chrono::system_clock::time_point(
            std::chrono::seconds(1560190138))));
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
  std::shared_ptr<MockClock> clock_;
};

TEST_F(DelegatedCredentialFactoryTest, TestCredentialParsing) {
  auto entry = generateEntry();
  auto cert = factory_.makePeerCert(std::move(entry), true);
  EXPECT_TRUE(cert);
  EXPECT_TRUE(typeid(*cert) == typeid(PeerDelegatedCredential<KeyType::P256>));
}

TEST_F(DelegatedCredentialFactoryTest, TestCredentialParsingNonLeaf) {
  auto entry = generateEntry();
  auto cert = factory_.makePeerCert(std::move(entry), false);
  EXPECT_TRUE(cert);
  EXPECT_TRUE(typeid(*cert) == typeid(OpenSSLPeerCertImpl<KeyType::P256>));
}

TEST_F(DelegatedCredentialFactoryTest, TestCredentialNoX509Extension) {
  // "Wrong" certs have different time, adjust to compensate.
  // Date: 06/27/2019 @ 7:32pm (UTC)
  EXPECT_CALL(*clock_, getCurrentTime())
      .WillOnce(Return(std::chrono::system_clock::time_point(
          std::chrono::seconds(1561663956))));
  auto entry = generateEntry();
  entry.cert_data = getCertData(kCertNoDelegatedExtension);
  // Note: factory doesn't do signature checks, so we expect this to fail due to
  // the x509 extension missing.
  expectThrows(
      [&]() { factory_.makePeerCert(std::move(entry), true); },
      "cert is missing DelegationUsage extension");
}

TEST_F(DelegatedCredentialFactoryTest, TestCredentialNoKeyUsage) {
  // "Wrong" certs have different time, adjust to compensate.
  // Date: 06/27/2019 @ 7:32pm (UTC)
  EXPECT_CALL(*clock_, getCurrentTime())
      .WillOnce(Return(std::chrono::system_clock::time_point(
          std::chrono::seconds(1561663956))));
  auto entry = generateEntry();
  entry.cert_data = getCertData(kCertNoKeyUsage);
  // Expect the DelegationUsage check to pass, but missing keyusage
  expectThrows(
      [&]() { factory_.makePeerCert(std::move(entry), true); },
      "cert is missing KeyUsage extension");
}

TEST_F(DelegatedCredentialFactoryTest, TestCredentialWrongKeyUsage) {
  // "Wrong" certs have different time, adjust to compensate.
  // Date: 06/27/2019 @ 7:32pm (UTC)
  EXPECT_CALL(*clock_, getCurrentTime())
      .WillOnce(Return(std::chrono::system_clock::time_point(
          std::chrono::seconds(1561663956))));
  auto entry = generateEntry();
  entry.cert_data = getCertData(kCertWrongKeyUsage);
  // This one has keyusage extension, but it lacks the digitalsignature usage
  expectThrows(
      [&]() { factory_.makePeerCert(std::move(entry), true); },
      "cert lacks digital signature key usage");
}

TEST_F(DelegatedCredentialFactoryTest, TestCredentialNoCertEntryExtension) {
  auto entry = generateEntry();
  entry.extensions.clear();
  auto cert = factory_.makePeerCert(std::move(entry), true);
  EXPECT_TRUE(cert);
  EXPECT_TRUE(typeid(*cert) == typeid(OpenSSLPeerCertImpl<KeyType::P256>));
}

TEST_F(DelegatedCredentialFactoryTest, TestCertExpiredCredential) {
  auto entry = generateEntry();
  auto credential = getExtension<DelegatedCredential>(entry.extensions);
  // Make it expire immediately after cert is valid
  credential->valid_time = 0;
  entry.extensions.clear();
  entry.extensions.push_back(encodeExtension(*credential));
  expectThrows(
      [&]() { factory_.makePeerCert(std::move(entry), true); },
      "credential is no longer valid");
}

TEST_F(DelegatedCredentialFactoryTest, TestCredentialValidForTooLong) {
  auto entry = generateEntry();
  auto credential = getExtension<DelegatedCredential>(entry.extensions);
  // Add an extra week to the valid time to make it longer than the max
  credential->valid_time += 604800;
  entry.extensions.clear();
  entry.extensions.push_back(encodeExtension(*credential));
  expectThrows(
      [&]() { factory_.makePeerCert(std::move(entry), true); },
      "credential validity is longer than a week from now");
}

} // namespace test
} // namespace extensions
} // namespace fizz
