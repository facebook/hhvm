/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/CertificateVerifier.h>

namespace fizz {

/**
 * InsecureAcceptAnyCertificate is a CertificateVerifier that accepts any cert
 * chain Basically a noop CertificateVerifier.
 */

class InsecureAcceptAnyCertificate : public CertificateVerifier {
 public:
  InsecureAcceptAnyCertificate() {}

  std::shared_ptr<const Cert> verify(
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs)
      const override {
    return certs.front();
  }

  std::vector<Extension> getCertificateRequestExtensions() const override {
    return std::vector<Extension>();
  }
};

/**
 * StoreCertificateChain is a CertificateVerifier decorator that stores the most
 * recent certificate chain that was presented to the client.
 *
 * It is not thread safe. Each Fizz connection should use a distinct instance oF
 * StoreCertificiateChain.
 */

class StoreCertificateChain : public CertificateVerifier {
 public:
  explicit StoreCertificateChain(
      std::unique_ptr<CertificateVerifier> delegateVerifier)
      : delegateVerifier_(std::move(delegateVerifier)) {}

  std::shared_ptr<const Cert> verify(
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs)
      const override {
    certs_ = certs;
    return delegateVerifier_->verify(certs);
  }

  std::vector<Extension> getCertificateRequestExtensions() const override {
    return delegateVerifier_->getCertificateRequestExtensions();
  }

  std::vector<std::shared_ptr<const fizz::PeerCert>> getCerts() const {
    return certs_;
  }

 private:
  mutable std::vector<std::shared_ptr<const fizz::PeerCert>> certs_;
  std::unique_ptr<CertificateVerifier> delegateVerifier_;
};
} // namespace fizz
