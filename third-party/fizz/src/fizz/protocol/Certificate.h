/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/fizz-config.h>
#include <fizz/record/Types.h>

#include <map>

#if FIZZ_CERTIFICATE_USE_OPENSSL_CERT
#include <folly/io/async/AsyncTransportCertificate.h>
#include <folly/io/async/ssl/OpenSSLTransportCertificate.h>

namespace fizz {
using Cert = folly::AsyncTransportCertificate;
using SelfCertBase = folly::OpenSSLTransportCertificate;
using PeerCertBase = folly::OpenSSLTransportCertificate;
using IdentityCertBase = Cert;
} // namespace fizz

#else
namespace fizz {
class Cert {
 public:
  virtual ~Cert() = default;
  virtual std::string getIdentity() const = 0;
  virtual std::optional<std::string> getDER() const = 0;
};

using SelfCertBase = Cert;
using PeerCertBase = Cert;
using IdentityCertBase = Cert;
} // namespace fizz
#endif

namespace fizz {

enum class CertificateVerifyContext {
  Server,
  Client,
  Authenticator,
  ClientDelegatedCredential,
  ServerDelegatedCredential
};

class IdentityCert : public IdentityCertBase {
 public:
  explicit IdentityCert(std::string identity);
  ~IdentityCert() override = default;

  std::string getIdentity() const override;

  std::optional<std::string> getDER() const override;

 private:
  std::string identity_;
};

class SelfCert : public SelfCertBase {
 public:
  virtual ~SelfCert() override = default;

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

class PeerCert : public PeerCertBase {
 public:
  virtual ~PeerCert() override = default;

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

class CertificateSerialization {
 public:
  /**
   * Given a `fizz::Cert` object, construct a bytestring representation of
   * the object that will be included in the serialized version of a PSK.
   *
   * If the serializer cannot serialize the `fizz::Cert` object, then
   * `serialize` should return `nullptr`.
   */
  virtual std::unique_ptr<folly::IOBuf> serialize(
      const fizz::Cert& cert) const = 0;

  /**
   * Given a bytestring representation of a certificate, attempt to reconstruct
   * the in-memory `fizz::Cert`.
   *
   * If the `CertificateSerialization` does not recognize the serialized
   * representation, then `deserialize` should return `nullptr`.
   */
  virtual std::shared_ptr<const fizz::Cert> deserialize(
      folly::ByteRange range) const = 0;

  virtual ~CertificateSerialization() = default;
};

} // namespace fizz
