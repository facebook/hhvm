/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/backend/openssl/certificate/OpenSSLPeerCertImpl.h>
#include <fizz/backend/openssl/certificate/OpenSSLSelfCertImpl.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/util/Logging.h>
#include <folly/ssl/OpenSSLCertUtils.h>
#include <openssl/bio.h>

namespace {
int getCurveName(EVP_PKEY* key) {
  auto ecKey = EVP_PKEY_get0_EC_KEY(key);
  if (ecKey) {
    return EC_GROUP_get_curve_name(EC_KEY_get0_group(ecKey));
  }
  return 0;
}
} // namespace

namespace fizz {
namespace openssl {

namespace detail {

folly::Optional<std::string> getIdentityFromX509(X509* x) {
  auto cn = folly::ssl::OpenSSLCertUtils::getCommonName(*x);
  if (cn.has_value()) {
    return std::move(cn).value();
  }

  return folly::ssl::OpenSSLCertUtils::getSubject(*x);
}
} // namespace detail

Status CertUtils::getCertMessage(
    CertificateMsg& ret,
    Error& err,
    const std::vector<folly::ssl::X509UniquePtr>& certs,
    Buf certificateRequestContext) {
  // compose the cert entry list
  std::vector<CertificateEntry> entries;
  for (const auto& cert : certs) {
    CertificateEntry entry;
    int len = i2d_X509(cert.get(), nullptr);
    if (len < 0) {
      return err.error("Error computing length");
    }
    entry.cert_data = folly::IOBuf::create(len);
    auto dataPtr = entry.cert_data->writableData();
    len = i2d_X509(cert.get(), &dataPtr);
    if (len < 0) {
      return err.error("Error converting cert to DER");
    }
    entry.cert_data->append(len);
    // TODO: add any extensions.
    entries.push_back(std::move(entry));
  }

  CertificateMsg msg;
  msg.certificate_request_context = std::move(certificateRequestContext);
  msg.certificate_list = std::move(entries);
  ret = std::move(msg);
  return Status::Success;
}

Status CertUtils::makePeerCert(
    std::unique_ptr<PeerCert>& ret,
    Error& err,
    folly::ByteRange range) {
  if (range.size() == 0) {
    return err.error("empty peer cert");
  }
  const unsigned char* begin = range.data();
  folly::ssl::X509UniquePtr cert(d2i_X509(nullptr, &begin, range.size()));
  if (!cert) {
    return err.error("could not read cert");
  }
  if (begin != range.data() + range.size()) {
    FIZZ_VLOG(1) << "Did not read to end of certificate";
  }
  return makePeerCert(ret, err, std::move(cert));
}
Status CertUtils::makePeerCert(
    std::unique_ptr<PeerCert>& ret,
    Error& err,
    Buf certData) {
  return makePeerCert(ret, err, certData->coalesce());
}

Status CertUtils::makePeerCert(
    std::unique_ptr<PeerCert>& ret,
    Error& err,
    folly::ssl::X509UniquePtr cert) {
  folly::ssl::EvpPkeyUniquePtr pubKey(X509_get_pubkey(cert.get()));
  if (!pubKey) {
    return err.error("couldn't get pubkey from peer cert");
  }
  const auto pkeyID = EVP_PKEY_id(pubKey.get());
  if (pkeyID == EVP_PKEY_RSA) {
    ret = std::make_unique<OpenSSLPeerCertImpl<KeyType::RSA>>(std::move(cert));
    return Status::Success;
  } else if (pkeyID == EVP_PKEY_EC) {
    switch (getCurveName(pubKey.get())) {
      case NID_X9_62_prime256v1:
        ret = std::make_unique<OpenSSLPeerCertImpl<KeyType::P256>>(
            std::move(cert));
        return Status::Success;
      case NID_secp384r1:
        ret = std::make_unique<OpenSSLPeerCertImpl<KeyType::P384>>(
            std::move(cert));
        return Status::Success;
      case NID_secp521r1:
        ret = std::make_unique<OpenSSLPeerCertImpl<KeyType::P521>>(
            std::move(cert));
        return Status::Success;
      default:
        break;
    }
  } else if (pkeyID == EVP_PKEY_ED25519) {
    ret = std::make_unique<OpenSSLPeerCertImpl<KeyType::ED25519>>(
        std::move(cert));
    return Status::Success;
  }
  return err.error("unknown peer cert type");
}

Status CertUtils::readPrivateKeyFromBuffer(
    folly::ssl::EvpPkeyUniquePtr& ret,
    Error& err,
    std::string keyData,
    char* password) {
  folly::ssl::BioUniquePtr b(BIO_new_mem_buf(
      const_cast<void*>( // needed by openssl 1.0.2d at least
          reinterpret_cast<const void*>(keyData.data())),
      keyData.size()));

  if (!b) {
    return err.error("failed to create BIO");
  }

  folly::ssl::EvpPkeyUniquePtr key(
      PEM_read_bio_PrivateKey(b.get(), nullptr, nullptr, password));

  if (!key) {
    return err.error("Failed to read key");
  }

  ret = std::move(key);
  return Status::Success;
}

namespace {

Status selfCertFromDataInternal(
    std::unique_ptr<SelfCert>& ret,
    Error& err,
    std::string certData,
    std::string keyData,
    char* password,
    const std::vector<std::shared_ptr<CertificateCompressor>>& compressors) {
  auto certs = folly::ssl::OpenSSLCertUtils::readCertsFromBuffer(
      folly::StringPiece(certData));
  if (certs.empty()) {
    return err.error("no certificates read");
  }

  folly::ssl::EvpPkeyUniquePtr key;
  FIZZ_RETURN_ON_ERROR(
      CertUtils::readPrivateKeyFromBuffer(
          key, err, std::move(keyData), password));

  return CertUtils::makeSelfCert(
      ret, err, std::move(certs), std::move(key), compressors);
}

} // namespace

Status CertUtils::makeSelfCert(
    std::unique_ptr<SelfCert>& ret,
    Error& err,
    std::string certData,
    std::string keyData,
    const std::vector<std::shared_ptr<CertificateCompressor>>& compressors) {
  return selfCertFromDataInternal(
      ret, err, std::move(certData), std::move(keyData), nullptr, compressors);
}

Status CertUtils::makeSelfCert(
    std::unique_ptr<SelfCert>& ret,
    Error& err,
    std::string certData,
    std::string encryptedKeyData,
    std::string password,
    const std::vector<std::shared_ptr<CertificateCompressor>>& compressors) {
  return selfCertFromDataInternal(
      ret,
      err,
      std::move(certData),
      std::move(encryptedKeyData),
      &password[0],
      compressors);
}

Status CertUtils::getKeyType(
    KeyType& ret,
    Error& err,
    const folly::ssl::EvpPkeyUniquePtr& key) {
  const auto pkeyID = EVP_PKEY_id(key.get());
  if (pkeyID == EVP_PKEY_RSA) {
    ret = KeyType::RSA;
    return Status::Success;
  } else if (pkeyID == EVP_PKEY_EC) {
    switch (getCurveName(key.get())) {
      case NID_X9_62_prime256v1:
        ret = KeyType::P256;
        return Status::Success;
      case NID_secp384r1:
        ret = KeyType::P384;
        return Status::Success;
      case NID_secp521r1:
        ret = KeyType::P521;
        return Status::Success;
    }
  } else if (pkeyID == EVP_PKEY_ED25519) {
    ret = KeyType::ED25519;
    return Status::Success;
  }

  return err.error("unknown key type");
}

Status CertUtils::getSigSchemes(
    std::vector<SignatureScheme>& ret,
    Error& err,
    KeyType type) {
  switch (type) {
    case KeyType::RSA:
      ret = getSigSchemes<KeyType::RSA>();
      return Status::Success;
    case KeyType::P256:
      ret = getSigSchemes<KeyType::P256>();
      return Status::Success;
    case KeyType::P384:
      ret = getSigSchemes<KeyType::P384>();
      return Status::Success;
    case KeyType::P521:
      ret = getSigSchemes<KeyType::P521>();
      return Status::Success;
    case KeyType::ED25519:
      ret = getSigSchemes<KeyType::ED25519>();
      return Status::Success;
  }

  return err.error("unknown key type");
}

Status CertUtils::makeSelfCert(
    std::unique_ptr<SelfCert>& ret,
    Error& err,
    std::vector<folly::ssl::X509UniquePtr> certs,
    folly::ssl::EvpPkeyUniquePtr key,
    const std::vector<std::shared_ptr<CertificateCompressor>>& compressors) {
  folly::ssl::EvpPkeyUniquePtr pubKey(X509_get_pubkey(certs.front().get()));
  if (!pubKey) {
    return err.error("Failed to read public key");
  }

  KeyType keyType;
  FIZZ_RETURN_ON_ERROR(getKeyType(keyType, err, pubKey));

  switch (keyType) {
    case KeyType::RSA:
      ret = std::make_unique<OpenSSLSelfCertImpl<KeyType::RSA>>(
          std::move(key), std::move(certs), compressors);
      return Status::Success;
    case KeyType::P256:
      ret = std::make_unique<OpenSSLSelfCertImpl<KeyType::P256>>(
          std::move(key), std::move(certs), compressors);
      return Status::Success;
    case KeyType::P384:
      ret = std::make_unique<OpenSSLSelfCertImpl<KeyType::P384>>(
          std::move(key), std::move(certs), compressors);
      return Status::Success;
    case KeyType::P521:
      ret = std::make_unique<OpenSSLSelfCertImpl<KeyType::P521>>(
          std::move(key), std::move(certs), compressors);
      return Status::Success;
    case KeyType::ED25519:
      ret = std::make_unique<OpenSSLSelfCertImpl<KeyType::ED25519>>(
          std::move(key), std::move(certs), compressors);
      return Status::Success;
  }

  return err.error("unknown self cert type");
}

CompressedCertificate CertUtils::cloneCompressedCert(
    const CompressedCertificate& src) {
  CompressedCertificate ret;
  ret.algorithm = src.algorithm;
  ret.compressed_certificate_message =
      src.compressed_certificate_message->clone();
  ret.uncompressed_length = src.uncompressed_length;
  return ret;
}

namespace {
class Serializer : public CertificateSerialization {
  std::unique_ptr<folly::IOBuf> serialize(
      const fizz::Cert& cert) const override {
    if (auto opensslCert =
            dynamic_cast<const folly::OpenSSLTransportCertificate*>(&cert)) {
      if (auto x509 = opensslCert->getX509()) {
        return folly::ssl::OpenSSLCertUtils::derEncode(*x509);
      }
    }
    return nullptr;
  }

  std::shared_ptr<const fizz::Cert> deserialize(
      folly::ByteRange range) const override {
    std::unique_ptr<PeerCert> ret;
    Error err;
    FIZZ_THROW_ON_ERROR(CertUtils::makePeerCert(ret, err, range), err);
    return ret;
  }
};
} // namespace

const CertificateSerialization& certificateSerializer() {
  static Serializer instance;
  return instance;
}

} // namespace openssl
} // namespace fizz
