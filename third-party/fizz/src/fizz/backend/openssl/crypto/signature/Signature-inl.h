/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/crypto/OpenSSLKeyUtils.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Assume.h>

namespace fizz {
namespace openssl {

namespace detail {

Status ecSign(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange data,
    const folly::ssl::EvpPkeyUniquePtr& pkey,
    int hashNid);

Status ecVerify(
    Error& err,
    folly::ByteRange data,
    folly::ByteRange signature,
    const folly::ssl::EvpPkeyUniquePtr& pkey,
    int hashNid);

Status edSign(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange data,
    const folly::ssl::EvpPkeyUniquePtr& pkey);

Status edVerify(
    Error& err,
    folly::ByteRange data,
    folly::ByteRange signature,
    const folly::ssl::EvpPkeyUniquePtr& pkey);

Status rsaPssSign(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange data,
    const folly::ssl::EvpPkeyUniquePtr& pkey,
    int hashNid);

Status rsaPssVerify(
    Error& err,
    folly::ByteRange data,
    folly::ByteRange signature,
    const folly::ssl::EvpPkeyUniquePtr& pkey,
    int hashNid);
} // namespace detail

template <SignatureScheme Scheme>
struct SigAlg {};

template <>
struct SigAlg<SignatureScheme::rsa_pss_sha256> {
  static constexpr int HashNid = NID_sha256;
  static constexpr KeyType type = KeyType::RSA;
};

template <>
struct SigAlg<SignatureScheme::ecdsa_secp256r1_sha256> {
  static constexpr int HashNid = NID_sha256;
  static constexpr KeyType type = KeyType::P256;
};

template <>
struct SigAlg<SignatureScheme::ecdsa_secp384r1_sha384> {
  static constexpr int HashNid = NID_sha384;
  static constexpr KeyType type = KeyType::P384;
};

template <>
struct SigAlg<SignatureScheme::ecdsa_secp521r1_sha512> {
  static constexpr int HashNid = NID_sha512;
  static constexpr KeyType type = KeyType::P521;
};

template <KeyType Type>
template <SignatureScheme Scheme>
inline std::unique_ptr<folly::IOBuf> OpenSSLSignature<Type>::sign(
    folly::ByteRange data) const {
  static_assert(
      SigAlg<Scheme>::type == Type, "Called with mismatched type and scheme");
  std::unique_ptr<folly::IOBuf> ret;
  Error err;
  switch (Type) {
    case KeyType::P256:
    case KeyType::P384:
    case KeyType::P521:
      FIZZ_THROW_ON_ERROR(
          detail::ecSign(ret, err, data, pkey_, SigAlg<Scheme>::HashNid), err);
      return ret;
    case KeyType::RSA:
      FIZZ_THROW_ON_ERROR(
          detail::rsaPssSign(ret, err, data, pkey_, SigAlg<Scheme>::HashNid),
          err);
      return ret;
    default:
      folly::assume_unreachable();
  }
}

// Use template specialization for Ed25519 because the algorithm doesn't have a
// HashNid and therefore its SigAlg struct would be missing a member that is
// used in the generic template
template <>
template <>
inline std::unique_ptr<folly::IOBuf>
OpenSSLSignature<KeyType::ED25519>::sign<SignatureScheme::ed25519>(
    folly::ByteRange data) const {
  std::unique_ptr<folly::IOBuf> ret;
  Error err;
  FIZZ_THROW_ON_ERROR(detail::edSign(ret, err, data, pkey_), err);
  return ret;
}

template <KeyType Type>
template <SignatureScheme Scheme>
inline void OpenSSLSignature<Type>::verify(
    folly::ByteRange data,
    folly::ByteRange signature) const {
  Error err;
  switch (Type) {
    case KeyType::P256:
    case KeyType::P384:
    case KeyType::P521:
      FIZZ_THROW_ON_ERROR(
          detail::ecVerify(
              err, data, signature, pkey_, SigAlg<Scheme>::HashNid),
          err);
      return;
    case KeyType::RSA:
      FIZZ_THROW_ON_ERROR(
          detail::rsaPssVerify(
              err, data, signature, pkey_, SigAlg<Scheme>::HashNid),
          err);
      return;
    case KeyType::ED25519:
    default:
      folly::assume_unreachable();
  }
}

// Use template specialization for Ed25519 because the algorithm doesn't have a
// HashNid and therefore its SigAlg struct would be missing a member that is
// used in the generic template
template <>
template <>
inline void
OpenSSLSignature<KeyType::ED25519>::verify<SignatureScheme::ed25519>(
    folly::ByteRange data,
    folly::ByteRange signature) const {
  Error err;
  FIZZ_THROW_ON_ERROR(detail::edVerify(err, data, signature, pkey_), err);
}

template <>
inline Status OpenSSLSignature<KeyType::P256>::setKey(
    Error& err,
    folly::ssl::EvpPkeyUniquePtr pkey) {
  FIZZ_RETURN_ON_ERROR(detail::validateECKey(err, pkey, NID_X9_62_prime256v1));
  pkey_ = std::move(pkey);
  return Status::Success;
}

template <>
inline Status OpenSSLSignature<KeyType::P384>::setKey(
    Error& err,
    folly::ssl::EvpPkeyUniquePtr pkey) {
  FIZZ_RETURN_ON_ERROR(detail::validateECKey(err, pkey, NID_secp384r1));
  pkey_ = std::move(pkey);
  return Status::Success;
}

template <>
inline Status OpenSSLSignature<KeyType::P521>::setKey(
    Error& err,
    folly::ssl::EvpPkeyUniquePtr pkey) {
  FIZZ_RETURN_ON_ERROR(detail::validateECKey(err, pkey, NID_secp521r1));
  pkey_ = std::move(pkey);
  return Status::Success;
}

template <>
inline Status OpenSSLSignature<KeyType::ED25519>::setKey(
    Error& err,
    folly::ssl::EvpPkeyUniquePtr pkey) {
  FIZZ_RETURN_ON_ERROR(detail::validateEdKey(err, pkey, NID_ED25519));
  pkey_ = std::move(pkey);
  return Status::Success;
}

template <>
inline Status OpenSSLSignature<KeyType::RSA>::setKey(
    Error& err,
    folly::ssl::EvpPkeyUniquePtr pkey) {
  if (EVP_PKEY_id(pkey.get()) != EVP_PKEY_RSA) {
    return err.error("key not rsa");
  }
  pkey_ = std::move(pkey);
  return Status::Success;
}

template <KeyType T>
inline folly::ssl::EvpPkeyUniquePtr OpenSSLSignature<T>::getKey() const {
  EVP_PKEY_up_ref(pkey_.get());
  return folly::ssl::EvpPkeyUniquePtr(pkey_.get());
}
} // namespace openssl
} // namespace fizz
