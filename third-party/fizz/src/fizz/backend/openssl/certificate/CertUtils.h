/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/signature/Signature.h>
#include <fizz/compression/CertificateCompressor.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/record/Types.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {

class SelfCert;
class PeerCert;
enum class CertificateVerifyContext;

namespace openssl {

class CertUtils {
 public:
  static Status getCertMessage(
      CertificateMsg& ret,
      Error& err,
      const std::vector<folly::ssl::X509UniquePtr>& certs,
      Buf certificateRequestContext);

  template <KeyType T>
  static std::vector<SignatureScheme> getSigSchemes();

  static Status
  getSigSchemes(std::vector<SignatureScheme>& ret, Error& err, KeyType type);

  /**
   * Create a PeerCert from the ASN1 encoded certData.
   */
  static Status
  makePeerCert(std::unique_ptr<PeerCert>& ret, Error& err, Buf certData);

  static Status makePeerCert(
      std::unique_ptr<PeerCert>& ret,
      Error& err,
      folly::ByteRange certData);

  /**
   * Create a PeerCert from a given X509
   */
  static Status makePeerCert(
      std::unique_ptr<PeerCert>& ret,
      Error& err,
      folly::ssl::X509UniquePtr cert);

  /**
   * Creates a SelfCert using the supplied certificate/key file data and
   * compressors.
   */
  static Status makeSelfCert(
      std::unique_ptr<SelfCert>& ret,
      Error& err,
      std::string certData,
      std::string keyData,
      const std::vector<std::shared_ptr<CertificateCompressor>>& compressors =
          {});

  static Status readPrivateKeyFromBuffer(
      folly::ssl::EvpPkeyUniquePtr& ret,
      Error& err,
      std::string keyData,
      char* password = nullptr);

  /**
   * Returns the key type for a public/private key.
   */
  static Status
  getKeyType(KeyType& ret, Error& err, const folly::ssl::EvpPkeyUniquePtr& key);

  /**
   * Creates a SelfCert using the supplied certificate, encrypted key data,
   * and password.
   */
  static Status makeSelfCert(
      std::unique_ptr<SelfCert>& ret,
      Error& err,
      std::string certData,
      std::string encryptedKeyData,
      std::string password,
      const std::vector<std::shared_ptr<CertificateCompressor>>& compressors =
          {});

  static Status makeSelfCert(
      std::unique_ptr<SelfCert>& ret,
      Error& err,
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

  template <KeyType T>
  static Status verify(
      Error& err,
      const OpenSSLSignature<T>& certSignature,
      SignatureScheme scheme,
      CertificateVerifyContext context,
      folly::ByteRange toBeSigned,
      folly::ByteRange signature);

  // Deprecated: use the Status-returning overloads above.
  static std::unique_ptr<PeerCert> makePeerCert(Buf certData);
  static std::unique_ptr<PeerCert> makePeerCert(folly::ByteRange certData);
  static std::unique_ptr<PeerCert> makePeerCert(folly::ssl::X509UniquePtr cert);

  // Deprecated: use the Status-returning overloads above.
  static std::unique_ptr<SelfCert> makeSelfCert(
      std::string certData,
      std::string keyData,
      const std::vector<std::shared_ptr<CertificateCompressor>>& compressors =
          {});
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

  // Deprecated: use the Status-returning overload above.
  static KeyType getKeyType(const folly::ssl::EvpPkeyUniquePtr& key);
};

const CertificateSerialization& certificateSerializer();

} // namespace openssl
} // namespace fizz

#include <fizz/backend/openssl/certificate/CertUtils-inl.h>
