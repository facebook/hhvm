/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <folly/ssl/OpenSSLCertUtils.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

#include <fizz/crypto/Utils.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>
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

class DelegatedCredentialUtilsTest : public Test {
 public:
  void SetUp() override {
    CryptoUtils::init();
    clock_ = std::make_shared<MockClock>();
    // Default return time to June 10, 2019 6:08:58 PM
    ON_CALL(*clock_, getCurrentTime())
        .WillByDefault(Return(std::chrono::system_clock::time_point(
            std::chrono::seconds(1560190138))));
  }

  void expectThrows(
      std::function<void()> f,
      std::string errorStr,
      AlertDescription alertType) {
    std::string what;
    folly::Optional<AlertDescription> desc;
    try {
      f();
    } catch (const FizzException& e) {
      what = e.what();
      desc = e.getAlert();
    }

    EXPECT_TRUE(desc.hasValue());
    EXPECT_EQ(desc.value(), alertType);
    EXPECT_THAT(what, HasSubstr(errorStr));
  }

  DelegatedCredential getCredential() {
    std::vector<Extension> exts;
    Extension ext;
    ext.extension_type = ExtensionType::delegated_credential;
    auto data = unhexlify(kDelegatedCred);
    ext.extension_data = folly::IOBuf::copyBuffer(data.data(), data.size());
    exts.push_back(std::move(ext));
    return *getExtension<DelegatedCredential>(exts);
  }

 protected:
  std::shared_ptr<MockClock> clock_;
};

TEST_F(DelegatedCredentialUtilsTest, TestCredentialNoX509Extension) {
  expectThrows(
      [&]() {
        DelegatedCredentialUtils::checkExtensions(
            getCert(kCertNoDelegatedExtension));
      },
      "cert is missing DelegationUsage extension",
      AlertDescription::illegal_parameter);
}

TEST_F(DelegatedCredentialUtilsTest, TestCredentialNoKeyUsage) {
  expectThrows(
      [&]() {
        DelegatedCredentialUtils::checkExtensions(getCert(kCertNoKeyUsage));
      },
      "cert is missing KeyUsage extension",
      AlertDescription::illegal_parameter);
}

TEST_F(DelegatedCredentialUtilsTest, TestCredentialWrongKeyUsage) {
  expectThrows(
      [&]() {
        DelegatedCredentialUtils::checkExtensions(getCert(kCertWrongKeyUsage));
      },
      "cert lacks digital signature key usage",
      AlertDescription::illegal_parameter);
}

TEST_F(DelegatedCredentialUtilsTest, TestGetCredentialExpiresTime) {
  auto cert = fizz::test::getCert(kCert);
  uint32_t secondsSinceEpoch = 1000000;
  auto notBeforeTime = std::chrono::system_clock::to_time_t(
      std::chrono::system_clock::time_point(
          std::chrono::seconds(secondsSinceEpoch)));
  folly::ssl::ASN1TimeUniquePtr notBeforeTimeASN1(
      ASN1_TIME_set(nullptr, notBeforeTime));
  X509_set1_notBefore(cert.get(), notBeforeTimeASN1.get());

  auto credential = getCredential();
  credential.valid_time = 234567;

  auto credentialExpiresTime =
      DelegatedCredentialUtils::getCredentialExpiresTime(cert, credential);
  EXPECT_EQ(
      credentialExpiresTime,
      std::chrono::system_clock::time_point(
          std::chrono::seconds(secondsSinceEpoch + credential.valid_time)));
}

TEST_F(DelegatedCredentialUtilsTest, TestCertExpiredCredential) {
  auto credential = getCredential();
  // Make it expire immediately after cert is valid
  credential.valid_time = 0;
  expectThrows(
      [&]() {
        DelegatedCredentialUtils::checkCredentialTimeValidity(
            fizz::test::getCert(kCert), credential, clock_);
      },
      "credential is no longer valid",
      AlertDescription::certificate_expired);
}

TEST_F(DelegatedCredentialUtilsTest, TestCredentialValidForTooLong) {
  auto credential = getCredential();
  // Add an extra week to the valid time to make it longer than the max
  credential.valid_time += 604800;
  expectThrows(
      [&]() {
        DelegatedCredentialUtils::checkCredentialTimeValidity(
            fizz::test::getCert(kCert), credential, clock_);
      },
      "credential validity is longer than a week from now",
      AlertDescription::illegal_parameter);
}

TEST_F(DelegatedCredentialUtilsTest, TestCertNotAfter) {
  auto cert = fizz::test::getCert(kCert);

  auto notBefore = X509_get0_notBefore(cert.get());
  auto notBeforeTime =
      folly::ssl::OpenSSLCertUtils::asnTimeToTimepoint(notBefore);
  auto notAfterTime = clock_->getCurrentTime() + std::chrono::days(1);
  auto notAfterTimeT = std::chrono::system_clock::to_time_t(notAfterTime);
  folly::ssl::ASN1TimeUniquePtr notAfterTimeASN1(
      ASN1_TIME_set(nullptr, notAfterTimeT));
  X509_set1_notAfter(cert.get(), notAfterTimeASN1.get());

  auto credentialExpiryTime = notAfterTime + std::chrono::seconds(1);
  auto credentialValidTime = std::chrono::duration_cast<std::chrono::seconds>(
      credentialExpiryTime - notBeforeTime);

  auto credential = getCredential();
  credential.valid_time = credentialValidTime.count();
  expectThrows(
      [&]() {
        DelegatedCredentialUtils::checkCredentialTimeValidity(
            cert, credential, clock_);
      },
      "credential validity is longer than parent cert validity",
      AlertDescription::illegal_parameter);
}
} // namespace test
} // namespace extensions
} // namespace fizz
