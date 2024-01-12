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
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {
class SelfCert;
class PeerCert;
enum class CertificateVerifyContext;

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
} // namespace fizz

#include <fizz/protocol/CertUtils-inl.h>
