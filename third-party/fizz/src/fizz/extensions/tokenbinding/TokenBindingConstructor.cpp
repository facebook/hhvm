/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

// Copyright 2004-present Facebook. All Rights Reserved.

#include <fizz/extensions/tokenbinding/TokenBindingConstructor.h>

#include <fizz/crypto/Sha256.h>
#include <fizz/crypto/openssl/OpenSSLKeyUtils.h>
#include <fizz/extensions/tokenbinding/Utils.h>

using namespace folly::ssl;

namespace fizz {
namespace extensions {

TokenBinding TokenBindingConstructor::createTokenBinding(
    EVP_PKEY& keyPair,
    const Buf& ekm,
    TokenBindingKeyParameters negotiatedParameters,
    TokenBindingType type) {
  if (negotiatedParameters != TokenBindingKeyParameters::ecdsap256) {
    throw std::runtime_error(folly::to<std::string>(
        "key params not implemented: ", negotiatedParameters));
  }

  EcKeyUniquePtr ecKey(EVP_PKEY_get1_EC_KEY(&keyPair));
  if (!ecKey) {
    throw std::runtime_error("Unable to retrieve EC Key");
  }

  TokenBinding binding;
  binding.tokenbinding_type = type;
  binding.extensions = folly::IOBuf::create(0);

  auto message =
      TokenBindingUtils::constructMessage(type, negotiatedParameters, ekm);
  binding.signature = signWithEcKey(ecKey, message);

  TokenBindingID id;
  id.key_parameters = negotiatedParameters;
  id.key = encodeEcKey(ecKey);
  binding.tokenbindingid = std::move(id);
  return binding;
}

Buf TokenBindingConstructor::signWithEcKey(
    const EcKeyUniquePtr& key,
    const Buf& message) {
  std::array<uint8_t, fizz::Sha256::HashLen> hashedMessage;
  fizz::Sha256::hash(
      *message,
      folly::MutableByteRange(hashedMessage.data(), hashedMessage.size()));

  EcdsaSigUniquePtr ecSignature(
      ECDSA_do_sign(hashedMessage.data(), hashedMessage.size(), key.get()));
  if (!ecSignature.get()) {
    throw std::runtime_error("Unable to sign message with EC Key");
  }

  return encodeEcdsaSignature(ecSignature);
}

Buf TokenBindingConstructor::encodeEcdsaSignature(
    const EcdsaSigUniquePtr& signature) {
  BIGNUM* r;
  BIGNUM* s;
  ECDSA_SIG_get0(signature.get(), (const BIGNUM**)&r, (const BIGNUM**)&s);
  if (!r || !s) {
    throw std::runtime_error("Unable to retrieve Bignum from ECDSA sig");
  }

  Buf encodedSignature =
      folly::IOBuf::create(TokenBindingUtils::kP256EcKeySize);
  addBignumToSignature(encodedSignature, r);
  addBignumToSignature(encodedSignature, s);
  return encodedSignature;
}

void TokenBindingConstructor::addBignumToSignature(
    const Buf& signature,
    BIGNUM* bigNum) {
  auto length = BN_num_bytes(bigNum);
  if (length > TokenBindingUtils::kP256EcKeySize / 2) {
    throw std::runtime_error("ECDSA sig bignum is of incorrect size");
  }
  // if a bignum is less than 32 bytes long, it has a most significant byte
  // of 0, so we have to pad the buffer
  size_t padding = (TokenBindingUtils::kP256EcKeySize / 2) - length;
  std::memset(signature->writableTail(), 0x00, padding);
  signature->append(padding);

  auto lenActual = BN_bn2bin(bigNum, signature->writableTail());
  signature->append(lenActual);
  if (lenActual != length) {
    throw std::runtime_error("bn2bin returned unexpected value");
  }
}

Buf TokenBindingConstructor::encodeEcKey(const EcKeyUniquePtr& ecKey) {
  auto ecKeyBuf = detail::encodeECPublicKey(ecKey);
  if (ecKeyBuf->isChained() ||
      ecKeyBuf->length() != TokenBindingUtils::kP256EcKeySize + 1) {
    throw std::runtime_error("Incorrect encoded EC Key Length");
  }
  ecKeyBuf->writableData()[0] = TokenBindingUtils::kP256EcKeySize;
  return ecKeyBuf;
}
} // namespace extensions
} // namespace fizz
