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

  Status verify(
      std::shared_ptr<const Cert>& ret,
      Error& /* err */,
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs)
      const override {
    ret = certs.front();
    return Status::Success;
  }

  Status getCertificateRequestExtensions(
      std::vector<fizz::Extension>& ret,
      fizz::Error& /* err */) const override {
    ret = std::vector<fizz::Extension>();
    return Status::Success;
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

  Status verify(
      std::shared_ptr<const Cert>& ret,
      Error& err,
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs)
      const override {
    certs_ = certs;
    return delegateVerifier_->verify(ret, err, certs);
  }

  Status getCertificateRequestExtensions(
      std::vector<Extension>& ret,
      Error& err) const override {
    return delegateVerifier_->getCertificateRequestExtensions(ret, err);
  }

  std::vector<std::shared_ptr<const fizz::PeerCert>> getCerts() const {
    return certs_;
  }

 private:
  mutable std::vector<std::shared_ptr<const fizz::PeerCert>> certs_;
  std::unique_ptr<CertificateVerifier> delegateVerifier_;
};
} // namespace fizz
