/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/certificate/OpenSSLPeerCertImpl.h>
#include <fizz/extensions/delegatedcred/Types.h>
#include <fizz/protocol/clock/SystemClock.h>

namespace fizz {
namespace extensions {

// This is a base class purely to differentiate between the common
// peer cert impl, in case a cast is needed at some higher layer.
class PeerDelegatedCredential : public PeerCert {
 public:
  virtual ~PeerDelegatedCredential() override = default;

  virtual const DelegatedCredential& getDelegatedCredential() const = 0;
};

template <openssl::KeyType T>
class PeerDelegatedCredentialImpl : public PeerDelegatedCredential {
 public:
  PeerDelegatedCredentialImpl(
      folly::ssl::X509UniquePtr cert,
      folly::ssl::EvpPkeyUniquePtr pubKey,
      DelegatedCredential credential);

  ~PeerDelegatedCredentialImpl() override = default;

  void verify(
      SignatureScheme scheme,
      CertificateVerifyContext context,
      folly::ByteRange toBeSigned,
      folly::ByteRange signature) const override;

  folly::ssl::X509UniquePtr getX509() const override {
    return peerCertImpl_.getX509();
  }

  std::string getIdentity() const override {
    return peerCertImpl_.getIdentity();
  }

  const DelegatedCredential& getDelegatedCredential() const override {
    return credential_;
  }

  SignatureScheme getExpectedScheme() const;

  /* for testing only */
  void setClock(std::shared_ptr<Clock> clock) {
    clock_ = clock;
  }

 private:
  class InternalPeerCert : public openssl::OpenSSLPeerCertImpl<T> {
   public:
    ~InternalPeerCert() override = default;

    explicit InternalPeerCert(
        folly::ssl::X509UniquePtr cert,
        folly::ssl::EvpPkeyUniquePtr pubKey);

    using openssl::OpenSSLPeerCertImpl<T>::signature_;
    using openssl::OpenSSLPeerCertImpl<T>::cert_;
  };
  InternalPeerCert peerCertImpl_;
  DelegatedCredential credential_;
  std::shared_ptr<Clock> clock_ = std::make_shared<SystemClock>();
};
} // namespace extensions
} // namespace fizz

#include <fizz/extensions/delegatedcred/PeerDelegatedCredential-inl.h>
