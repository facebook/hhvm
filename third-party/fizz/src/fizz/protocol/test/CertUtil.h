/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/certificate/OpenSSLPeerCertImpl.h>

namespace fizz {
namespace test {

struct CertAndKey {
  folly::ssl::X509UniquePtr cert;
  folly::ssl::EvpPkeyUniquePtr key;
};

template <typename A, typename B>
void throwIfNeq(const A& a, const B& b, const std::string& msg) {
  if (a != b) {
    throw std::runtime_error(msg);
  }
}

template <typename A>
void throwIfNull(const A& a, const std::string& msg) {
  if (a == nullptr) {
    throw std::runtime_error(msg);
  }
}

struct CreateCertOptions {
  std::string cn;
  bool ca{false};
  CertAndKey* issuer{nullptr};
  std::optional<std::chrono::system_clock::time_point> notBefore;
  std::optional<std::chrono::system_clock::time_point> notAfter;
};

inline folly::ssl::ASN1TimeUniquePtr asn1(
    std::chrono::system_clock::time_point tp) {
  folly::ssl::ASN1TimeUniquePtr ret(ASN1_TIME_new());
  ASN1_TIME_set(ret.get(), std::chrono::system_clock::to_time_t(tp));
  return ret;
}

inline CertAndKey createCert(CreateCertOptions options) {
  const auto& cn = options.cn;
  bool ca = options.ca;
  const auto& issuer = options.issuer;

  folly::ssl::EvpPkeyUniquePtr pk(EVP_PKEY_new());
  throwIfNull(pk, "private key creation failed");

  folly::ssl::X509UniquePtr crt(X509_new());
  throwIfNull(crt, "cert creation failed");

  folly::ssl::EcGroupUniquePtr grp(
      EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1));
  throwIfNull(grp, "group creation failed");

  EC_GROUP_set_asn1_flag(grp.get(), OPENSSL_EC_NAMED_CURVE);
  EC_GROUP_set_point_conversion_form(grp.get(), POINT_CONVERSION_UNCOMPRESSED);

  throwIfNeq(EC_GROUP_check(grp.get(), nullptr), 1, "group check failed");

  folly::ssl::EcKeyUniquePtr ec(EC_KEY_new());
  throwIfNull(ec, "ec key creation error");

  throwIfNeq(EC_KEY_set_group(ec.get(), grp.get()), 1, "failed to set group");

  throwIfNeq(EC_KEY_generate_key(ec.get()), 1, "ec generation failed");

  throwIfNeq(
      EVP_PKEY_set1_EC_KEY(pk.get(), ec.get()),
      1,
      "private key assignment failed");

  X509_set_version(crt.get(), 2);
  static int serial = 0;
  ASN1_INTEGER_set(X509_get_serialNumber(crt.get()), serial++);
  using clock = std::chrono::system_clock;

  auto notBefore = clock::now();
  if (options.notBefore.has_value()) {
    notBefore = *options.notBefore;
  }

  auto notAfter = notBefore + std::chrono::seconds(31536000);
  if (options.notAfter.has_value()) {
    notAfter = *options.notAfter;
  }

  X509_set_notBefore(crt.get(), asn1(notBefore).get());
  X509_set_notAfter(crt.get(), asn1(notAfter).get());

  throwIfNeq(
      X509_set_pubkey(crt.get(), pk.get()), 1, "public key assignment failed");

  X509_NAME* name = X509_get_subject_name(crt.get());
  const std::vector<std::pair<std::string, std::string>> entries{
      {"C", "US"}, {"O", "Facebook, Inc."}, {"CN", cn}};
  for (const auto& entry : entries) {
    throwIfNeq(
        X509_NAME_add_entry_by_txt(
            name,
            entry.first.c_str(),
            MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>(entry.second.c_str()),
            -1,
            -1,
            0),
        1,
        std::string("failed to set name entry: ") + entry.first);
  }

  X509V3_CTX ctx;
  if (issuer) {
    X509V3_set_ctx(&ctx, issuer->cert.get(), crt.get(), nullptr, nullptr, 0);
  } else {
    X509V3_set_ctx(&ctx, crt.get(), crt.get(), nullptr, nullptr, 0);
  }

  std::string configuration = R"(
[fizz]
subjectKeyIdentifier    = hash
authorityKeyIdentifier  = keyid:always, issuer
extendedKeyUsage        = critical, serverAuth, clientAuth
)";
  if (ca) {
    configuration.append("basicConstraints = critical, CA:TRUE\n");
    configuration.append(
        "keyUsage = critical, cRLSign, digitalSignature, keyCertSign\n");
  } else {
    configuration.append("basicConstraints = critical, CA:FALSE\n");
    configuration.append("keyUsage = critical, cRLSign, digitalSignature\n");
  }

  folly::ssl::BioUniquePtr bio(
      BIO_new_mem_buf(configuration.data(), configuration.size()));
  CONF* c = NCONF_new(nullptr);
  NCONF_load_bio(c, bio.get(), nullptr);
  // Create X509_EXTENSIONS from each extension config specified in the "fizz"
  // section then add them into the X509 `crt`
  X509V3_EXT_add_nconf(c, &ctx, "fizz", crt.get());
  NCONF_free(c);

  if (issuer) {
    throwIfNeq(
        X509_set_issuer_name(
            crt.get(), X509_get_subject_name(issuer->cert.get())),
        1,
        "failed to set issuer");
    if (X509_sign(crt.get(), issuer->key.get(), EVP_sha256()) == 0) {
      throw std::runtime_error("failed to sign certificate");
    }
  } else {
    throwIfNeq(
        X509_set_issuer_name(crt.get(), name), 1, "failed to set issuer");
    if (X509_sign(crt.get(), pk.get(), EVP_sha256()) == 0) {
      throw std::runtime_error("failed to self-sign certificate");
    }
  }

  return {std::move(crt), std::move(pk)};
}

inline CertAndKey createCert(std::string cn, bool ca, CertAndKey* issuer) {
  return createCert({.cn = std::move(cn), .ca = ca, .issuer = issuer});
}

inline std::shared_ptr<PeerCert> getPeerCert(const CertAndKey& cert) {
  return std::make_shared<openssl::OpenSSLPeerCertImpl<openssl::KeyType::P256>>(
      folly::ssl::X509UniquePtr(X509_dup(cert.cert.get())));
}

} // namespace test
} // namespace fizz
