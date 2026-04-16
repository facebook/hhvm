/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

// Copyright 2004-present Facebook. All Rights Reserved.

#include <fizz/extensions/tokenbinding/TokenBindingConstructor.h>

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/extensions/tokenbinding/Utils.h>

using namespace folly::ssl;

namespace fizz {
namespace extensions {

Status TokenBindingConstructor::createTokenBinding(
    TokenBinding& ret,
    Error& err,
    EVP_PKEY& keyPair,
    const Buf& ekm,
    TokenBindingKeyParameters negotiatedParameters,
    TokenBindingType type) {
  if (negotiatedParameters != TokenBindingKeyParameters::ecdsap256) {
    return err.error(
        folly::to<std::string>(
            "key params not implemented: ", negotiatedParameters));
  }

  EcKeyUniquePtr ecKey(EVP_PKEY_get1_EC_KEY(&keyPair));
  if (!ecKey) {
    return err.error("Unable to retrieve EC Key");
  }

  TokenBinding binding;
  binding.tokenbinding_type = type;
  binding.extensions = folly::IOBuf::create(0);

  auto message =
      TokenBindingUtils::constructMessage(type, negotiatedParameters, ekm);
  FIZZ_RETURN_ON_ERROR(signWithEcKey(binding.signature, err, ecKey, message));

  TokenBindingID id;
  id.key_parameters = negotiatedParameters;
  FIZZ_RETURN_ON_ERROR(encodeEcKey(id.key, err, ecKey));
  binding.tokenbindingid = std::move(id);
  ret = std::move(binding);
  return Status::Success;
}

Status TokenBindingConstructor::signWithEcKey(
    Buf& ret,
    Error& err,
    const EcKeyUniquePtr& key,
    const Buf& message) {
  std::array<uint8_t, fizz::Sha256::HashLen> hashedMessage;
  fizz::hash(
      fizz::openssl::hasherFactory<fizz::Sha256>(),
      *message,
      folly::MutableByteRange(hashedMessage.data(), hashedMessage.size()));

  EcdsaSigUniquePtr ecSignature(
      ECDSA_do_sign(hashedMessage.data(), hashedMessage.size(), key.get()));
  if (!ecSignature.get()) {
    return err.error("Unable to sign message with EC Key");
  }

  return encodeEcdsaSignature(ret, err, ecSignature);
}

Status TokenBindingConstructor::encodeEcdsaSignature(
    Buf& ret,
    Error& err,
    const EcdsaSigUniquePtr& signature) {
  BIGNUM* r;
  BIGNUM* s;
  ECDSA_SIG_get0(signature.get(), (const BIGNUM**)&r, (const BIGNUM**)&s);
  if (!r || !s) {
    return err.error("Unable to retrieve Bignum from ECDSA sig");
  }

  Buf encodedSignature =
      folly::IOBuf::create(TokenBindingUtils::kP256EcKeySize);
  FIZZ_RETURN_ON_ERROR(addBignumToSignature(err, encodedSignature, r));
  FIZZ_RETURN_ON_ERROR(addBignumToSignature(err, encodedSignature, s));
  ret = std::move(encodedSignature);
  return Status::Success;
}

Status TokenBindingConstructor::addBignumToSignature(
    Error& err,
    const Buf& signature,
    BIGNUM* bigNum) {
  auto length = BN_num_bytes(bigNum);
  if (length > TokenBindingUtils::kP256EcKeySize / 2) {
    return err.error("ECDSA sig bignum is of incorrect size");
  }
  // if a bignum is less than 32 bytes long, it has a most significant byte
  // of 0, so we have to pad the buffer
  size_t padding = (TokenBindingUtils::kP256EcKeySize / 2) - length;
  std::memset(signature->writableTail(), 0x00, padding);
  signature->append(padding);

  auto lenActual = BN_bn2bin(bigNum, signature->writableTail());
  signature->append(lenActual);
  if (lenActual != length) {
    return err.error("bn2bin returned unexpected value");
  }
  return Status::Success;
}

Status TokenBindingConstructor::encodeEcKey(
    Buf& ret,
    Error& err,
    const EcKeyUniquePtr& ecKey) {
  auto ecKeyBuf = openssl::detail::encodeECPublicKey(ecKey);
  if (ecKeyBuf->isChained() ||
      ecKeyBuf->length() != TokenBindingUtils::kP256EcKeySize + 1) {
    return err.error("Incorrect encoded EC Key Length");
  }
  ecKeyBuf->writableData()[0] = TokenBindingUtils::kP256EcKeySize;
  ret = std::move(ecKeyBuf);
  return Status::Success;
}
} // namespace extensions
} // namespace fizz
