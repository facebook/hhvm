/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/experimental/crypto/BatchSignature.h>
#include <fizz/experimental/protocol/BatchSignatureTypes.h>
#include <fizz/protocol/Certificate.h>

namespace fizz {

/**
 * A decorator class for an exisiting PeerCert to support both
 * its existing signature schemes and corresponding batch signature schemes.
 */
class BatchSignaturePeerCert : public PeerCert {
 public:
  explicit BatchSignaturePeerCert(std::shared_ptr<const PeerCert> verifier)
      : verifier_(verifier) {}

  std::string getIdentity() const override {
    return verifier_->getIdentity();
  }

  folly::ssl::X509UniquePtr getX509() const override {
    return verifier_->getX509();
  }

  void verify(
      SignatureScheme scheme,
      CertificateVerifyContext context,
      folly::ByteRange message,
      folly::ByteRange signature) const override {
    auto baseScheme = getBatchSchemeInfo(scheme);
    if (!baseScheme) {
      verifier_->verify(scheme, context, message, signature);
      return;
    }
    batchSigVerify(scheme, baseScheme.value(), context, message, signature);
  }

  /**
   * Get the base PeerCert.
   */
  std::shared_ptr<const PeerCert> getVerifier() const {
    return verifier_;
  }

 private:
  // TODO: the current implementation only support BatchSignature with Hash
  // function Sha256.
  /**
   * Verify a batch signature.
   *
   * - Check signature encoding.
   * - Check validity of index value and path length.
   * - Verify the correctness of the signature value.
   */
  void batchSigVerify(
      SignatureScheme scheme,
      BatchSchemeInfo batchInfo,
      CertificateVerifyContext context,
      folly::ByteRange message,
      folly::ByteRange signature) const;

  std::shared_ptr<const PeerCert> verifier_;
};

} // namespace fizz
