/*
 *  Copyright (c) Facebook, Inc. and its affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <fizz/extensions/delegatedcred/Types.h>
#include <fizz/protocol/OpenSSLSelfCertImpl.h>

namespace fizz {
namespace extensions {

// This is a base "interface" for testing/mock purposes.
class SelfDelegatedCredential : public SelfCert {
 public:
  virtual ~SelfDelegatedCredential() = default;
  virtual const DelegatedCredential& getDelegatedCredential() const = 0;
};

// Self cert implementation of delegated credentials. This is for servers to
// authenticate themselves using delegated credentials.
//
// The inheritance is a bit funny cause we want to derive from SelfCert directly
// to inherit the pure virtual base (for tests) but we also want the
// implementation to inherit from OpenSSLSelfCertImpl (to share logic). To
// achieve that without diamond inheritance, the implementation class derives
// from the interface, and it has an internal private class that derives from
// the corresponding OpenSSLSelfCertImpl class to provide the implementation
// logic.
template <KeyType T>
class SelfDelegatedCredentialImpl : public SelfDelegatedCredential {
 public:
  ~SelfDelegatedCredentialImpl() override = default;

  SelfDelegatedCredentialImpl(
      std::vector<folly::ssl::X509UniquePtr> certs,
      folly::ssl::EvpPkeyUniquePtr privateKey,
      DelegatedCredential credential,
      const std::vector<std::shared_ptr<CertificateCompressor>>& compressors =
          {});

  std::string getIdentity() const override;

  std::vector<std::string> getAltIdentities() const override;

  std::vector<SignatureScheme> getSigSchemes() const override;

  CertificateMsg getCertMessage(
      Buf certificateRequestContext = nullptr) const override;

  CompressedCertificate getCompressedCert(
      CertificateCompressionAlgorithm algo) const override;

  Buf sign(
      SignatureScheme scheme,
      CertificateVerifyContext context,
      folly::ByteRange toBeSigned) const override;

  folly::ssl::X509UniquePtr getX509() const override;

  const DelegatedCredential& getDelegatedCredential() const override;

 private:
  class InternalSelfCert : public OpenSSLSelfCertImpl<T> {
   public:
    ~InternalSelfCert() override = default;

    InternalSelfCert(
        std::vector<folly::ssl::X509UniquePtr> certs,
        folly::ssl::EvpPkeyUniquePtr privateKey);
    using OpenSSLSelfCertImpl<T>::certs_;
    using OpenSSLSelfCertImpl<T>::signature_;
  };
  InternalSelfCert selfCertImpl_;
  DelegatedCredential credential_;
  std::map<CertificateCompressionAlgorithm, CompressedCertificate>
      compressedCerts_;
};
} // namespace extensions
} // namespace fizz

#include <fizz/extensions/delegatedcred/SelfDelegatedCredential-inl.h>
