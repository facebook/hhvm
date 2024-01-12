/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/OpenSSLPeerCertImpl.h>

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

inline CertAndKey createCert(std::string cn, bool ca, CertAndKey* issuer) {
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
  X509_gmtime_adj(X509_get_notBefore(crt.get()), 0);
  X509_gmtime_adj(X509_get_notAfter(crt.get()), 31536000);

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

  if (ca) {
    X509V3_CTX ctx;
    X509V3_set_ctx_nodb(&ctx);
    std::array<char, 8> constraint{"CA:TRUE"};
    folly::ssl::X509ExtensionUniquePtr ext(X509V3_EXT_conf_nid(
        nullptr, &ctx, NID_basic_constraints, constraint.data()));
    throwIfNull(ext, "failed to create extension");
    throwIfNeq(
        X509_EXTENSION_set_critical(ext.get(), 1), 1, "failed to set critical");
    throwIfNeq(
        X509_add_ext(crt.get(), ext.get(), -1), 1, "failed to add extension");
  }

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

inline std::shared_ptr<PeerCert> getPeerCert(const CertAndKey& cert) {
  return std::make_shared<OpenSSLPeerCertImpl<KeyType::P256>>(
      folly::ssl::X509UniquePtr(X509_dup(cert.cert.get())));
}

} // namespace test
} // namespace fizz
