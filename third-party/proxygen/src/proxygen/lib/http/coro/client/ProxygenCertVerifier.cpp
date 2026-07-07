/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/ProxygenCertVerifier.h"

#include <fmt/core.h>
#include <folly/String.h>
#include <folly/portability/OpenSSL.h>
#include <folly/ssl/OpenSSLCertUtils.h>

namespace proxygen::coro {
namespace {
static std::string getErrorMsg(X509* leaf, const std::string& expected) {
  auto sans = folly::ssl::OpenSSLCertUtils::getSubjectAltNames(*leaf);
  return fmt::format(
      "certificate identity verification failed: expected={}, "
      "cert_sans=[{}]",
      expected,
      folly::join(", ", sans));
}

// A fizz::CertificateVerifier wrapper that adds identity (hostname/IP)
// verification on top of an inner verifier. Exposed only through
// makeVerifier().
class ProxygenCertVerifier : public fizz::CertificateVerifier {
 public:
  ProxygenCertVerifier(
      std::shared_ptr<const fizz::CertificateVerifier> verifier,
      ExpectedIdentity expectedIdentity,
      ValidationPolicy policy)
      : verifier_(std::move(verifier)),
        expectedIdentity_(std::move(expectedIdentity)),
        policy_(policy) {
    CHECK(verifier_);
  }

  // On top of the inner verifier, this also verifies that the leaf cert
  // matches the expected identity.
  fizz::Status verify(std::shared_ptr<const fizz::Cert>& ret,
                      fizz::Error& err,
                      const std::vector<std::shared_ptr<const fizz::PeerCert>>&
                          certs) const override {
    if (certs.empty()) {
      return err.error("no certificates to verify");
    }
    auto leafCert = certs.front()->getX509();
    if (leafCert == nullptr) {
      return err.error("peer certificate has no X509 certificate");
    }

    FIZZ_RETURN_ON_ERROR(verifier_->verify(ret, err, certs));
    FIZZ_RETURN_ON_ERROR(hostnameCheck(err, leafCert.get()));
    FIZZ_RETURN_ON_ERROR(ipCheck(err, leafCert.get()));
    return fizz::Status::Success;
  }

  // Delegates to the inner verifier.
  fizz::Status getCertificateRequestExtensions(
      std::vector<fizz::Extension>& ret, fizz::Error& err) const override {
    return verifier_->getCertificateRequestExtensions(ret, err);
  }

 private:
  fizz::Status hostnameCheck(fizz::Error& err, X509* leaf) const {
    auto hostname = expectedIdentity_.getHostname();
    if (!hostname.has_value()) {
      return fizz::Status::Success;
    }
    if (X509_check_host(leaf,
                        hostname->c_str(),
                        hostname->size(),
                        X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS,
                        nullptr) == 1) {
      return fizz::Status::Success;
    }
    if (policy_ != ValidationPolicy::Enforcing) {
      return fizz::Status::Success;
    }
    return err.error(getErrorMsg(leaf, *hostname),
                     fizz::AlertDescription::bad_certificate,
                     fizz::Error::Category::Verifier);
  }

  fizz::Status ipCheck(fizz::Error& err, X509* leaf) const {
    auto ip = expectedIdentity_.getIp();
    if (!ip.has_value()) {
      return fizz::Status::Success;
    }
    auto ipStr = ip->str();
    if (X509_check_ip_asc(leaf, ipStr.c_str(), 0) == 1) {
      return fizz::Status::Success;
    }
    if (policy_ != ValidationPolicy::Enforcing) {
      return fizz::Status::Success;
    }
    return err.error(getErrorMsg(leaf, ipStr),
                     fizz::AlertDescription::bad_certificate,
                     fizz::Error::Category::Verifier);
  }

  std::shared_ptr<const fizz::CertificateVerifier> verifier_;
  ExpectedIdentity expectedIdentity_;
  ValidationPolicy policy_;
};
} // namespace

ExpectedIdentity ExpectedIdentity::expectIP(folly::IPAddress ip) {
  return ExpectedIdentity{std::move(ip)};
}

ExpectedIdentity ExpectedIdentity::expectDNS(std::string hostname) {
  return ExpectedIdentity{std::move(hostname)};
}

std::optional<std::string> ExpectedIdentity::getHostname() const {
  if (std::holds_alternative<std::string>(identity)) {
    return std::get<std::string>(identity);
  }
  return std::nullopt;
}

std::optional<folly::IPAddress> ExpectedIdentity::getIp() const {
  if (std::holds_alternative<folly::IPAddress>(identity)) {
    return std::get<folly::IPAddress>(identity);
  }
  return std::nullopt;
}

std::shared_ptr<fizz::CertificateVerifier> makeVerifier(
    std::shared_ptr<const fizz::CertificateVerifier> verifier,
    ExpectedIdentity expectedIdentity,
    ValidationPolicy policy) {
  return std::make_shared<ProxygenCertVerifier>(
      std::move(verifier), std::move(expectedIdentity), policy);
}
} // namespace proxygen::coro
