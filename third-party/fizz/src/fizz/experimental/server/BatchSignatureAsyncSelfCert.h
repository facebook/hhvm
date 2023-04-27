/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/experimental/batcher/Batcher.h>
#include <fizz/experimental/crypto/BatchSignature.h>
#include <fizz/experimental/protocol/BatchSignatureTypes.h>
#include <fizz/server/AsyncSelfCert.h>
#include <fizz/server/State.h>

namespace fizz {

/**
 * A decorator class for an exisiting SelfCert/AsyncSelfCert to support both
 * its existing signature schemes and corresponding batch signature schemes.
 */
template <typename Hash = Sha256>
class BatchSignatureAsyncSelfCert : public AsyncSelfCert {
 public:
  /**
   * Construct a batch BatchSignatureAsyncSelfCert instance.
   *
   * @param signer  the existing SelfCert/AsyncSelfCert.
   * @param batcher the pointer to the manager of the underlying Merkle
   *                tree.
   */
  BatchSignatureAsyncSelfCert(std::shared_ptr<Batcher<Hash>> batcher)
      : signer_(batcher->getSigner()), batcher_(batcher) {}

  std::string getIdentity() const override {
    return signer_->getIdentity();
  }

  std::vector<std::string> getAltIdentities() const override {
    return signer_->getAltIdentities();
  }

  fizz::CertificateMsg getCertMessage(
      fizz::Buf certificateRequestContext = nullptr) const override {
    return signer_->getCertMessage(std::move(certificateRequestContext));
  }

  std::vector<SignatureScheme> getSigSchemes() const override {
    auto result = signer_->getSigSchemes();
    result.push_back(batcher_->getBatchScheme());
    return result;
  }

  fizz::CompressedCertificate getCompressedCert(
      fizz::CertificateCompressionAlgorithm algo) const override {
    return signer_->getCompressedCert(algo);
  }

  folly::ssl::X509UniquePtr getX509() const override {
    return signer_->getX509();
  }

  std::unique_ptr<folly::IOBuf> sign(
      fizz::SignatureScheme scheme,
      fizz::CertificateVerifyContext context,
      folly::ByteRange toBeSigned) const override {
    // delegate all sign() to signer_ because
    // batch scheme is only supported with signFuture()
    return signer_->sign(scheme, context, toBeSigned);
  }

  folly::SemiFuture<folly::Optional<Buf>> signFuture(
      SignatureScheme scheme,
      CertificateVerifyContext context,
      std::unique_ptr<folly::IOBuf> message) const override {
    // if the scheme is the supported batch scheme, use the signer to sign it
    // directly
    if (scheme != batcher_->getBatchScheme()) {
      auto asyncSigner = dynamic_cast<const AsyncSelfCert*>(signer_.get());
      if (asyncSigner) {
        return asyncSigner->signFuture(scheme, context, std::move(message));
      } else {
        return signer_->sign(scheme, context, message->coalesce());
      }
    }
    // otherwise, generate batch signature with batcher
    return batchSigSign(message->coalesce());
  }

  /**
   * Get the base SelfCert.
   */
  std::shared_ptr<const SelfCert> getSigner() const {
    return signer_;
  }

 private:
  folly::SemiFuture<folly::Optional<Buf>> batchSigSign(
      folly::ByteRange message) const {
    // Add message into the merkle tree and get the root value and path
    auto batchResult = batcher_->addMessageAndSign(message);
    auto index = batchResult.index_;
    return std::move(batchResult.future_).deferValue([=](auto&& signedTree) {
      BatchSignature sig(
          signedTree.tree_->getPath(index),
          folly::IOBuf::wrapBuffer(
              signedTree.signature_->data(), signedTree.signature_->size()));
      return folly::Optional(sig.encode());
    });
  }

  std::shared_ptr<const SelfCert> signer_;
  std::shared_ptr<Batcher<Hash>> batcher_;
};

} // namespace fizz
