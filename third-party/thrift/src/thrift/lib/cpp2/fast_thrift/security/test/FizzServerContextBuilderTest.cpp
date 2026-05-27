/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>

#include <array>
#include <cstdio>
#include <fstream>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <fizz/fizz-config.h>
#include <fizz/protocol/CertificateVerifier.h>
#include <fizz/protocol/DefaultCertificateVerifier.h>
#include <fizz/protocol/DefaultFactory.h>
#include <fizz/record/Types.h>
#include <fizz/server/TicketTypes.h>

#include <thrift/lib/cpp2/fast_thrift/security/test/TestCert.h>

namespace apache::thrift::fast_thrift::security::test {

namespace {

// Writes `contents` to a fresh temp file and returns its path. The file is
// removed when the returned guard goes out of scope.
class TempPemFile {
 public:
  explicit TempPemFile(const std::string& contents) {
    std::array<char, sizeof("/tmp/fast_thrift_test_pem_XXXXXX")> tmpl{
        "/tmp/fast_thrift_test_pem_XXXXXX"};
    int fd = mkstemp(tmpl.data());
    CHECK_GE(fd, 0) << "mkstemp failed";
    path_ = tmpl.data();
    std::ofstream out(path_);
    out << contents;
    out.close();
    ::close(fd);
  }

  TempPemFile(const TempPemFile&) = delete;
  TempPemFile& operator=(const TempPemFile&) = delete;

  ~TempPemFile() {
    if (!path_.empty()) {
      std::remove(path_.c_str());
    }
  }

  const std::string& path() const { return path_; }

 private:
  std::string path_;
};

// Trivial CertificateVerifier subclass for identity-equality checks. Never
// invoked by the tests in this file (we only assert that
// buildTLSParams hands the same shared_ptr through to the context).
class NoopCertVerifier : public fizz::CertificateVerifier {
 public:
  fizz::Status verify(
      std::shared_ptr<const fizz::Cert>& /*ret*/,
      fizz::Error& /*err*/,
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& /*certs*/)
      const override {
    return fizz::Status::Success;
  }
  fizz::Status getCertificateRequestExtensions(
      std::vector<fizz::Extension>& /*ret*/,
      fizz::Error& /*err*/) const override {
    return fizz::Status::Success;
  }
};

} // namespace

class FizzServerContextBuilderTest : public ::testing::Test {
 protected:
  TestCert cert_ = makeTestCert();
  ThriftTlsConfig thriftConfig_{};

  // Convenience: cert/key set, clientAuth=None (the default-cert-load
  // path that doesn't trigger verifier requirements).
  FizzServerCertConfig baseConfigNoClientAuth() const {
    FizzServerCertConfig config;
    config.certPem = cert_.certPem;
    config.keyPem = cert_.keyPem;
    config.clientAuth = fizz::server::ClientAuthMode::None;
    return config;
  }
};

// === Cases 1, 5: defaults & ALPN mode ====================================

TEST_F(FizzServerContextBuilderTest, BuildsFromPemBuffersNoClientAuth) {
  auto config = baseConfigNoClientAuth();

  auto built = buildTLSParams(config, thriftConfig_);
  ASSERT_NE(built.fizzContext, nullptr);
  EXPECT_EQ(
      built.fizzContext->getSupportedAlpns(), std::vector<std::string>{"rs"});
  EXPECT_EQ(
      built.fizzContext->getClientAuthMode(),
      fizz::server::ClientAuthMode::None);
  EXPECT_EQ(built.fizzContext->getClientCertVerifier(), nullptr);
  // Default Optional ALPN mode (stricter than fizz's natural AllowMismatch).
  EXPECT_EQ(built.fizzContext->getAlpnMode(), fizz::server::AlpnMode::Optional);
  EXPECT_EQ(built.thriftParams, nullptr);
}

TEST_F(FizzServerContextBuilderTest, AppliesCustomAlpnAndAuthMode) {
  auto config = baseConfigNoClientAuth();
  config.alpnProtocols = {"h2", "rs"};

  auto built = buildTLSParams(config, thriftConfig_);
  ASSERT_NE(built.fizzContext, nullptr);
  EXPECT_EQ(
      built.fizzContext->getSupportedAlpns(),
      (std::vector<std::string>{"h2", "rs"}));
}

TEST_F(FizzServerContextBuilderTest, AlpnModeAllowMismatch) {
  auto config = baseConfigNoClientAuth();
  config.alpnMode = fizz::server::AlpnMode::AllowMismatch;

  auto built = buildTLSParams(config, thriftConfig_);
  EXPECT_EQ(
      built.fizzContext->getAlpnMode(), fizz::server::AlpnMode::AllowMismatch);
}

TEST_F(FizzServerContextBuilderTest, StopTlsBuildsThriftParamsContext) {
  auto config = baseConfigNoClientAuth();

  ThriftTlsConfig thriftConfig;
  thriftConfig.enableStopTLS = true;

  auto built = buildTLSParams(config, thriftConfig);
  ASSERT_NE(built.fizzContext, nullptr);
  ASSERT_NE(built.thriftParams, nullptr);
  EXPECT_TRUE(built.thriftParams->getUseStopTLS());
}

// === Cases 2-4: verifier resolution ======================================

TEST_F(
    FizzServerContextBuilderTest, ClientAuthRequiredWithCaPathBuildsVerifier) {
  auto chain = makeTestCertChain();
  TempPemFile caFile(chain.caCertPem);

  FizzServerCertConfig config;
  config.certPem = chain.leafCertPem;
  config.keyPem = chain.leafKeyPem;
  config.clientAuth = fizz::server::ClientAuthMode::Required;
  config.caPath = caFile.path();

  auto built = buildTLSParams(config, thriftConfig_);
  auto verifier = built.fizzContext->getClientCertVerifier();
  ASSERT_NE(verifier, nullptr);
  // DefaultCertificateVerifier aliases to OpenSSLCertificateVerifier when
  // the OpenSSL backend is on (the only build mode this test runs in).
  EXPECT_NE(
      dynamic_cast<const fizz::DefaultCertificateVerifier*>(verifier.get()),
      nullptr);
}

TEST_F(FizzServerContextBuilderTest, ClientAuthRequiredWithoutVerifierThrows) {
  FizzServerCertConfig config;
  config.certPem = cert_.certPem;
  config.keyPem = cert_.keyPem;
  config.clientAuth = fizz::server::ClientAuthMode::Required;

  try {
    buildTLSParams(config, thriftConfig_);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& ex) {
    EXPECT_THAT(ex.what(), ::testing::HasSubstr("Required or Optional"));
    EXPECT_THAT(ex.what(), ::testing::HasSubstr("caPath"));
    EXPECT_THAT(ex.what(), ::testing::HasSubstr("customClientCertVerifier"));
  }
}

TEST_F(FizzServerContextBuilderTest, ClientAuthOptionalWithoutVerifierThrows) {
  // Optional must throw for the same reason Required does: with no verifier
  // configured, fizz would still accept any peer cert without verification,
  // silently weakening the security posture the embedder asked for.
  FizzServerCertConfig config;
  config.certPem = cert_.certPem;
  config.keyPem = cert_.keyPem;
  config.clientAuth = fizz::server::ClientAuthMode::Optional;

  try {
    buildTLSParams(config, thriftConfig_);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& ex) {
    EXPECT_THAT(ex.what(), ::testing::HasSubstr("Required or Optional"));
    EXPECT_THAT(ex.what(), ::testing::HasSubstr("caPath"));
    EXPECT_THAT(ex.what(), ::testing::HasSubstr("customClientCertVerifier"));
  }
}

TEST_F(FizzServerContextBuilderTest, CustomVerifierTakesPrecedenceOverCaPath) {
  // Even with caPath set, customClientCertVerifier should win and the CA
  // file should not be opened (so a bogus path is fine).
  FizzServerCertConfig config;
  config.certPem = cert_.certPem;
  config.keyPem = cert_.keyPem;
  config.clientAuth = fizz::server::ClientAuthMode::Required;
  config.caPath = "/this/path/does/not/exist.pem";
  auto custom = std::make_shared<NoopCertVerifier>();
  config.customClientCertVerifier = custom;

  auto built = buildTLSParams(config, thriftConfig_);
  EXPECT_EQ(built.fizzContext->getClientCertVerifier().get(), custom.get());
}

// === Cases 6-7: AEGIS ====================================================

TEST_F(FizzServerContextBuilderTest, EnableAegisOff) {
  auto config = baseConfigNoClientAuth();
  config.enableAegis = false;

  auto built = buildTLSParams(config, thriftConfig_);
  for (const auto& group : built.fizzContext->getSupportedCiphers()) {
    for (auto cipher : group) {
      EXPECT_NE(cipher, fizz::CipherSuite::TLS_AEGIS_128L_SHA256);
    }
  }
}

#if FIZZ_HAVE_LIBAEGIS
TEST_F(FizzServerContextBuilderTest, EnableAegisOnPrependsCipher) {
  auto config = baseConfigNoClientAuth();
  config.enableAegis = true;

  auto built = buildTLSParams(config, thriftConfig_);
  const auto& ciphers = built.fizzContext->getSupportedCiphers();
  ASSERT_FALSE(ciphers.empty());
  ASSERT_FALSE(ciphers.front().empty());
  EXPECT_EQ(ciphers.front().front(), fizz::CipherSuite::TLS_AEGIS_128L_SHA256);
}
#endif

// === Case 8: supported groups ============================================

TEST_F(FizzServerContextBuilderTest, SupportedGroupsIncludeClassical) {
  auto config = baseConfigNoClientAuth();

  auto built = buildTLSParams(config, thriftConfig_);
  const auto& groups = built.fizzContext->getSupportedGroups();
  EXPECT_NE(
      std::find(groups.begin(), groups.end(), fizz::NamedGroup::x25519),
      groups.end());
  EXPECT_NE(
      std::find(groups.begin(), groups.end(), fizz::NamedGroup::secp256r1),
      groups.end());
}

#if FIZZ_HAVE_OQS && OQS_ENABLE_KEM_ml_kem_768
TEST_F(FizzServerContextBuilderTest, SupportedGroupsIncludeMLKEM) {
  auto config = baseConfigNoClientAuth();

  auto built = buildTLSParams(config, thriftConfig_);
  const auto& groups = built.fizzContext->getSupportedGroups();
  EXPECT_NE(
      std::find(groups.begin(), groups.end(), fizz::NamedGroup::X25519MLKEM768),
      groups.end());
  EXPECT_NE(
      std::find(
          groups.begin(), groups.end(), fizz::NamedGroup::X25519MLKEM512_FB),
      groups.end());
}
#endif

// === Cases 9-10: TLS versions and fallback ==============================

TEST_F(FizzServerContextBuilderTest, SupportedVersionsMatchExpected) {
  auto config = baseConfigNoClientAuth();

  auto built = buildTLSParams(config, thriftConfig_);
  EXPECT_EQ(
      built.fizzContext->getSupportedVersions(),
      (std::vector<fizz::ProtocolVersion>{
          fizz::ProtocolVersion::tls_1_3,
          fizz::ProtocolVersion::tls_1_3_28,
          fizz::ProtocolVersion::tls_1_3_26}));
}

TEST_F(FizzServerContextBuilderTest, VersionFallbackExplicitlyDisabled) {
  auto config = baseConfigNoClientAuth();

  auto built = buildTLSParams(config, thriftConfig_);
  EXPECT_FALSE(built.fizzContext->getVersionFallbackEnabled());
}

// === Case 11: customFactory ==============================================

TEST_F(FizzServerContextBuilderTest, CustomFactoryWired) {
  auto config = baseConfigNoClientAuth();
  auto factory = std::make_shared<fizz::DefaultFactory>();
  config.customFactory = factory;

  auto built = buildTLSParams(config, thriftConfig_);
  EXPECT_EQ(built.fizzContext->getFactoryPtr().get(), factory.get());
}

TEST_F(FizzServerContextBuilderTest, NoCustomFactoryUsesDefault) {
  auto config = baseConfigNoClientAuth();

  auto built = buildTLSParams(config, thriftConfig_);
  // FizzServerContext's default ctor wires a DefaultFactory; we never call
  // setFactory when customFactory is unset.
  EXPECT_NE(built.fizzContext->getFactoryPtr(), nullptr);
}

// === Cases 12-14: ticket cipher ==========================================

TEST_F(FizzServerContextBuilderTest, NoTicketSeedsNoCipher) {
  auto config = baseConfigNoClientAuth();

  auto built = buildTLSParams(config, thriftConfig_);
  EXPECT_EQ(built.fizzContext->getTicketCipher(), nullptr);
}

TEST_F(FizzServerContextBuilderTest, TicketSeedsIdentityOnly) {
  auto config = baseConfigNoClientAuth();
  TicketCipherSeeds seeds;
  seeds.currentSeeds = {
      "fakeSecretttttttttttttttttttttttttt"}; // 35 chars > 32 required
  seeds.kind = TicketCipherKind::IdentityOnly;
  config.ticketSeeds = seeds;

  auto built = buildTLSParams(config, thriftConfig_);
  const auto* cipher = built.fizzContext->getTicketCipher();
  ASSERT_NE(cipher, nullptr);
  EXPECT_NE(
      dynamic_cast<const fizz::server::AES128TicketIdentityOnlyCipher*>(cipher),
      nullptr);
}

TEST_F(FizzServerContextBuilderTest, TicketSeedsX509) {
  auto config = baseConfigNoClientAuth();
  TicketCipherSeeds seeds;
  seeds.currentSeeds = {"fakeSecretttttttttttttttttttttttttt"};
  seeds.kind = TicketCipherKind::X509;
  config.ticketSeeds = seeds;

  auto built = buildTLSParams(config, thriftConfig_);
  const auto* cipher = built.fizzContext->getTicketCipher();
  ASSERT_NE(cipher, nullptr);
  EXPECT_NE(
      dynamic_cast<const fizz::server::AES128TicketCipher*>(cipher), nullptr);
}

// === Case 15: OpenSSL backend guard for caPath ===========================

#if !FIZZ_CERTIFICATE_USE_OPENSSL_CERT
TEST_F(FizzServerContextBuilderTest, CaPathThrowsWithoutOpenSslBackend) {
  FizzServerCertConfig config;
  config.certPem = cert_.certPem;
  config.keyPem = cert_.keyPem;
  config.clientAuth = fizz::server::ClientAuthMode::Required;
  config.caPath = "/some/path.pem";

  try {
    buildTLSParams(config, thriftConfig_);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& ex) {
    EXPECT_THAT(
        ex.what(), ::testing::HasSubstr("FIZZ_CERTIFICATE_USE_OPENSSL_CERT"));
  }
}
#endif

// === Existing input-validation tests (preserved) =========================

TEST_F(FizzServerContextBuilderTest, RejectsBothPathAndBufferSet) {
  FizzServerCertConfig config;
  config.certPath = "/dev/null";
  config.keyPath = "/dev/null";
  config.certPem = cert_.certPem;
  config.keyPem = cert_.keyPem;
  config.clientAuth = fizz::server::ClientAuthMode::None;

  EXPECT_THROW(buildTLSParams(config, thriftConfig_), std::runtime_error);
}

TEST_F(FizzServerContextBuilderTest, RejectsNeitherPathNorBufferSet) {
  FizzServerCertConfig config;
  config.clientAuth = fizz::server::ClientAuthMode::None;
  EXPECT_THROW(buildTLSParams(config, thriftConfig_), std::runtime_error);
}

TEST_F(FizzServerContextBuilderTest, RejectsCertPathWithoutKeyPath) {
  FizzServerCertConfig config;
  config.certPath = "/tmp/whatever.pem";
  config.clientAuth = fizz::server::ClientAuthMode::None;
  EXPECT_THROW(buildTLSParams(config, thriftConfig_), std::runtime_error);
}

TEST_F(FizzServerContextBuilderTest, RejectsCertPemWithoutKeyPem) {
  FizzServerCertConfig config;
  config.certPem = cert_.certPem;
  config.clientAuth = fizz::server::ClientAuthMode::None;
  EXPECT_THROW(buildTLSParams(config, thriftConfig_), std::runtime_error);
}

TEST_F(FizzServerContextBuilderTest, ThrowsOnUnreadableCertPath) {
  FizzServerCertConfig config;
  config.certPath = "/this/path/does/not/exist.pem";
  config.keyPath = "/this/path/does/not/exist.key";
  config.clientAuth = fizz::server::ClientAuthMode::None;
  EXPECT_THROW(buildTLSParams(config, thriftConfig_), std::runtime_error);
}

TEST_F(FizzServerContextBuilderTest, ThrowsOnMalformedPem) {
  FizzServerCertConfig config;
  config.certPem = "not actually a PEM";
  config.keyPem = "also not a PEM";
  config.clientAuth = fizz::server::ClientAuthMode::None;
  EXPECT_THROW(buildTLSParams(config, thriftConfig_), std::runtime_error);
}

} // namespace apache::thrift::fast_thrift::security::test
