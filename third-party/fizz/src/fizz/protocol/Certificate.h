/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/compression/CertificateCompressor.h>
#include <fizz/crypto/signature/Signature.h>
#include <fizz/record/Types.h>
#include <folly/io/async/AsyncTransportCertificate.h>
#include <folly/io/async/ssl/OpenSSLTransportCertificate.h>

#include <map>

namespace fizz {

enum class CertificateVerifyContext {
  Server,
  Client,
  Authenticator,
  DelegatedCredential
};

using Cert = folly::AsyncTransportCertificate;
using OpenSSLCert = folly::OpenSSLTransportCertificate;

class IdentityCert : public Cert {
 public:
  explicit IdentityCert(std::string identity);
  ~IdentityCert() override = default;

  std::string getIdentity() const override;

 private:
  std::string identity_;
};

class SelfCert : public OpenSSLCert {
 public:
  ~SelfCert() override = default;

  /**
   * Returns additional identities this certificate can also represent (for
   * example subject alternate names).
   */
  virtual std::vector<std::string> getAltIdentities() const = 0;

  /**
   * Returns the signature schemes this certificate can be used with.
   */
  virtual std::vector<SignatureScheme> getSigSchemes() const = 0;

  virtual CertificateMsg getCertMessage(
      Buf certificateRequestContext = nullptr) const = 0;

  virtual CompressedCertificate getCompressedCert(
      CertificateCompressionAlgorithm algo) const = 0;

  virtual Buf sign(
      SignatureScheme scheme,
      CertificateVerifyContext context,
      folly::ByteRange toBeSigned) const = 0;
};

class PeerCert : public OpenSSLCert {
 public:
  ~PeerCert() override = default;

  /**
   * Verifies that signature is a valid signature of toBeSigned. Throws if it's
   * not.
   */
  virtual void verify(
      SignatureScheme scheme,
      CertificateVerifyContext context,
      folly::ByteRange toBeSigned,
      folly::ByteRange signature) const = 0;
};

class CertUtils {
 public:
  /**
   * Adds the appropriate context data to prepare toBeSigned for a signature
   * scheme's signing function.
   */
  static Buf prepareSignData(
      CertificateVerifyContext context,
      folly::ByteRange toBeSigned);

  static CertificateMsg getCertMessage(
      const std::vector<folly::ssl::X509UniquePtr>& certs,
      Buf certificateRequestContext);

  template <KeyType T>
  static std::vector<SignatureScheme> getSigSchemes();

  static std::vector<SignatureScheme> getSigSchemes(KeyType type);

  /**
   * Create a PeerCert from the ASN1 encoded certData.
   */
  static std::unique_ptr<PeerCert> makePeerCert(Buf certData);

  /**
   * Create a PeerCert from a given X509
   */
  static std::unique_ptr<PeerCert> makePeerCert(folly::ssl::X509UniquePtr cert);

  /**
   * Creates a SelfCert using the supplied certificate/key file data and
   * compressors.
   * Throws std::runtime_error on error.
   */
  static std::unique_ptr<SelfCert> makeSelfCert(
      std::string certData,
      std::string keyData,
      const std::vector<std::shared_ptr<CertificateCompressor>>& compressors =
          {});

  static folly::ssl::EvpPkeyUniquePtr readPrivateKeyFromBuffer(
      std::string keyData,
      char* password = nullptr);

  /**
   * Returns the key type for a public/private key.
   */
  static KeyType getKeyType(const folly::ssl::EvpPkeyUniquePtr& key);

  /**
   * Creates a SelfCert using the supplied certificate, encrypted key data,
   * and password. Throws std::runtime_error on error.
   */
  static std::unique_ptr<SelfCert> makeSelfCert(
      std::string certData,
      std::string encryptedKeyData,
      std::string password,
      const std::vector<std::shared_ptr<CertificateCompressor>>& compressors =
          {});

  static std::unique_ptr<SelfCert> makeSelfCert(
      std::vector<folly::ssl::X509UniquePtr> certs,
      folly::ssl::EvpPkeyUniquePtr key,
      const std::vector<std::shared_ptr<CertificateCompressor>>& compressors =
          {});

  /**
   * Clones a compressed cert by copying the relevant fields and cloning the
   * underlying data IOBuf.
   */
  static CompressedCertificate cloneCompressedCert(
      const CompressedCertificate& src);
};

template <KeyType T>
class SelfCertImpl : public SelfCert {
 public:
  /**
   * Private key is the private key associated with the leaf cert.
   * certs is a list of certs in the chain with the leaf first.
   */
  SelfCertImpl(
      folly::ssl::EvpPkeyUniquePtr pkey,
      std::vector<folly::ssl::X509UniquePtr> certs,
      const std::vector<std::shared_ptr<fizz::CertificateCompressor>>&
          compressors = {});

  ~SelfCertImpl() override = default;

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

 protected:
  // Allows derived classes to handle init
  explicit SelfCertImpl(std::vector<folly::ssl::X509UniquePtr> certs);
  OpenSSLSignature<T> signature_;
  std::vector<folly::ssl::X509UniquePtr> certs_;
  std::map<CertificateCompressionAlgorithm, CompressedCertificate>
      compressedCerts_;
};

template <KeyType T>
class PeerCertImpl : public PeerCert {
 public:
  explicit PeerCertImpl(folly::ssl::X509UniquePtr cert);

  ~PeerCertImpl() override = default;

  std::string getIdentity() const override;

  void verify(
      SignatureScheme scheme,
      CertificateVerifyContext context,
      folly::ByteRange toBeSigned,
      folly::ByteRange signature) const override;

  folly::ssl::X509UniquePtr getX509() const override;

 protected:
  OpenSSLSignature<T> signature_;
  folly::ssl::X509UniquePtr cert_;
};

} // namespace fizz

#include <fizz/protocol/Certificate-inl.h>
