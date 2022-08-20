/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/openssl/OpenSSLKeyUtils.h>

#include <openssl/err.h>

namespace fizz {

/*static*/ folly::ssl::EvpPkeyUniquePtr OpenSSLKeyUtils::generateECKeyPair(
    int curveNid) {
  return detail::generateECKeyPair(curveNid);
}

namespace detail {

void validateECKey(const folly::ssl::EvpPkeyUniquePtr& key, int curveNid) {
  folly::ssl::EcKeyUniquePtr ecKey(EVP_PKEY_get1_EC_KEY(key.get()));
  if (!ecKey) {
    throw std::runtime_error("Wrong key type");
  }
  if (EC_KEY_check_key(ecKey.get()) != 1) {
    throw std::runtime_error("Private key not valid");
  }
  folly::ssl::EcGroupUniquePtr curve(EC_GROUP_new_by_curve_name(curveNid));
  if (!curve) {
    throw std::runtime_error("Failed to create curve");
  }
  auto keyGroup = EC_KEY_get0_group(ecKey.get());
  if (EC_GROUP_cmp(keyGroup, curve.get(), nullptr) != 0) {
    throw std::runtime_error("Invalid group");
  }
}

#if FIZZ_OPENSSL_HAS_ED25519
void validateEdKey(const folly::ssl::EvpPkeyUniquePtr& key, int curveNid) {
  int pkeyNid = EVP_PKEY_base_id(key.get());
  if (pkeyNid != NID_ED25519 && pkeyNid != NID_ED448) {
    throw std::runtime_error("Wrong key type");
  }
  if (pkeyNid != curveNid) {
    throw std::runtime_error("Invalid group");
  }
}
#endif

std::unique_ptr<folly::IOBuf> generateEvpSharedSecret(
    const folly::ssl::EvpPkeyUniquePtr& key,
    const folly::ssl::EvpPkeyUniquePtr& peerKey) {
  folly::ssl::EvpPkeyCtxUniquePtr ctx(EVP_PKEY_CTX_new(key.get(), nullptr));
  if (EVP_PKEY_derive_init(ctx.get()) != 1) {
    throw std::runtime_error("Initializing derive context failed");
  }
  // Start deriving the key.
  if (EVP_PKEY_derive_set_peer(ctx.get(), peerKey.get()) != 1) {
    throw std::runtime_error("Error setting peer key");
  }
  size_t secretLen = 0;
  if (EVP_PKEY_derive(ctx.get(), nullptr, &secretLen) != 1) {
    throw std::runtime_error("Error deriving key");
  }
  // secretLen is now the maximum secret length.
  auto buf = folly::IOBuf::create(secretLen);
  if (EVP_PKEY_derive(ctx.get(), buf->writableData(), &secretLen) != 1) {
    throw std::runtime_error("Error deriving key");
  }
  buf->append(secretLen);
  return buf;
}

folly::ssl::EvpPkeyUniquePtr generateECKeyPair(int curveNid) {
  folly::ssl::EcKeyUniquePtr ecParamKey(EC_KEY_new_by_curve_name(curveNid));
  folly::ssl::EvpPkeyUniquePtr params(EVP_PKEY_new());
  if (!ecParamKey || !params) {
    throw std::runtime_error("Error initializing params");
  }
  if (EVP_PKEY_set1_EC_KEY(params.get(), ecParamKey.get()) != 1) {
    throw std::runtime_error("Error setting ec key for params");
  }
  folly::ssl::EvpPkeyCtxUniquePtr kctx(EVP_PKEY_CTX_new(params.get(), nullptr));
  if (!kctx) {
    throw std::runtime_error("Error creating kctx");
  }
  if (EVP_PKEY_keygen_init(kctx.get()) != 1) {
    throw std::runtime_error("Error initializing ctx");
  }
  EVP_PKEY* pkey = nullptr;
  if (EVP_PKEY_keygen(kctx.get(), &pkey) != 1) {
    throw std::runtime_error("Error generating key");
  }
  folly::ssl::EvpPkeyUniquePtr evpKey(pkey);
  folly::ssl::EcKeyUniquePtr ecKey(EVP_PKEY_get1_EC_KEY(evpKey.get()));
  validateECKey(evpKey, curveNid);
  return evpKey;
}

folly::ssl::EvpPkeyUniquePtr decodeECPublicKey(
    folly::ByteRange range,
    int curveNid) {
  // Get the peer key.
  folly::ssl::EcGroupUniquePtr curve(EC_GROUP_new_by_curve_name(curveNid));
  folly::ssl::EcKeyUniquePtr peerKey(EC_KEY_new_by_curve_name(curveNid));
  if (!curve || !peerKey) {
    throw std::runtime_error("Error initializing peer key");
  }
  folly::ssl::EcPointUniquePtr point(EC_POINT_new(curve.get()));
  if (!point) {
    throw std::runtime_error("Error initializing point");
  }
  if (EC_POINT_oct2point(
          curve.get(), point.get(), range.data(), range.size(), nullptr) != 1) {
    throw std::runtime_error("Error decoding peer key");
  }
  if (EC_POINT_is_on_curve(curve.get(), point.get(), nullptr) != 1) {
    throw std::runtime_error("Peer key is not on curve");
  }
  if (!EC_KEY_set_public_key(peerKey.get(), point.get())) {
    throw std::runtime_error("Error setting public key");
  }
  folly::ssl::EvpPkeyUniquePtr peerPkey(EVP_PKEY_new());
  if (EVP_PKEY_assign_EC_KEY(peerPkey.get(), peerKey.release()) != 1) {
    throw std::runtime_error("Error assigning EC key");
  }
  return peerPkey;
}

std::unique_ptr<folly::IOBuf> encodeECPublicKey(
    const folly::ssl::EvpPkeyUniquePtr& key) {
  folly::ssl::EcKeyUniquePtr ecKey(EVP_PKEY_get1_EC_KEY(key.get()));
  if (!ecKey) {
    throw std::runtime_error("Wrong key type");
  }
  return encodeECPublicKey(ecKey);
}

std::unique_ptr<folly::IOBuf> encodeECPublicKey(
    const folly::ssl::EcKeyUniquePtr& ecKey) {
  auto point = EC_KEY_get0_public_key(ecKey.get());
  auto group = EC_KEY_get0_group(ecKey.get());

  size_t len = EC_POINT_point2oct(
      group, point, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr);
  auto buf = folly::IOBuf::create(len);
  // TLS 1.3 only allows uncompressed point formats, so we only support that
  // for now.
  len = EC_POINT_point2oct(
      group,
      point,
      POINT_CONVERSION_UNCOMPRESSED,
      buf->writableData(),
      len,
      nullptr);
  if (len == 0) {
    throw std::runtime_error("Failed to encode key");
  }
  buf->append(len);
  return buf;
}

std::string getOpenSSLError() {
  auto err = ERR_get_error();
  if (err == 0) {
    return "";
  }
  char errMsg[256];
  ERR_error_string_n(ERR_get_error(), errMsg, sizeof(errMsg));
  return std::string(errMsg);
}
} // namespace detail
} // namespace fizz
