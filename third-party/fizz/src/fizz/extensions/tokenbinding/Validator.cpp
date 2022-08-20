/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/tokenbinding/Validator.h>

#include <fizz/crypto/Sha256.h>
#include <fizz/crypto/openssl/OpenSSLKeyUtils.h>
#include <fizz/extensions/tokenbinding/Utils.h>

using namespace folly;
using namespace folly::io;
using namespace folly::ssl;

namespace fizz {
namespace extensions {

Optional<TokenBindingID> Validator::validateTokenBinding(
    TokenBinding tokenBinding,
    const Buf& ekm,
    const TokenBindingKeyParameters& negotiatedParameters) {
  if (tokenBinding.tokenbindingid.key_parameters != negotiatedParameters) {
    VLOG(2) << "sent parameters: "
            << toString(tokenBinding.tokenbindingid.key_parameters)
            << " don't match negotiated parameters: "
            << toString(negotiatedParameters);
    return folly::none;
  }
  return constructAndVerifyMessage(std::move(tokenBinding), ekm);
}

Optional<TokenBindingID> Validator::validateTokenBinding(
    TokenBinding tokenBinding,
    const Buf& ekm,
    const std::vector<TokenBindingKeyParameters>& supportedParameters) {
  if (find(
          supportedParameters.begin(),
          supportedParameters.end(),
          tokenBinding.tokenbindingid.key_parameters) ==
      supportedParameters.end()) {
    VLOG(2) << "Supported key parameters (" << join(", ", supportedParameters)
            << ") does not include client's key parameter ("
            << toString(tokenBinding.tokenbindingid.key_parameters) << ")";
    return folly::none;
  }
  return constructAndVerifyMessage(std::move(tokenBinding), ekm);
}

Optional<TokenBindingID> Validator::constructAndVerifyMessage(
    TokenBinding tokenBinding,
    const Buf& ekm) {
  try {
    auto message = TokenBindingUtils::constructMessage(
        tokenBinding.tokenbinding_type,
        tokenBinding.tokenbindingid.key_parameters,
        ekm);
    verify(
        tokenBinding.tokenbindingid.key_parameters,
        tokenBinding.tokenbindingid.key,
        tokenBinding.signature,
        message);
    return std::move(tokenBinding.tokenbindingid);
  } catch (const std::exception& e) {
    VLOG(1) << "Token Binding Verification Failed: " << e.what();
    return folly::none;
  }
}

void Validator::verify(
    const TokenBindingKeyParameters& keyParams,
    const Buf& key,
    const Buf& signature,
    const Buf& message) {
  if (keyParams == TokenBindingKeyParameters::ecdsap256) {
    auto pkey = constructEcKeyFromBuf(key);
    auto ecdsa = constructECDSASig(signature);

    std::array<uint8_t, fizz::Sha256::HashLen> hashedMessage;
    fizz::Sha256::hash(
        *message,
        folly::MutableByteRange(hashedMessage.data(), hashedMessage.size()));
    if (ECDSA_do_verify(
            hashedMessage.data(),
            hashedMessage.size(),
            ecdsa.get(),
            pkey.get()) != 1) {
      throw std::runtime_error(folly::to<std::string>(
          "Verification failed: ", detail::getOpenSSLError()));
    }
#if FIZZ_OPENSSL_HAS_ED25519
  } else if (keyParams == TokenBindingKeyParameters::ed25519_experimental) {
    // Read the first byte from `key`, which denotes the size of the key
    Cursor keyReader(key.get());
    auto keyLen = keyReader.readBE<uint8_t>();

    // Verify that the key size matches the size of an Ed25519 key
    if (keyLen != TokenBindingUtils::kEd25519KeySize) {
      throw std::runtime_error(
          folly::to<std::string>("Incorrect key size for Ed25519: ", keyLen));
    }

    // Instantiate a EvpPkeyUniquePtr from the rest of the bytes
    auto keyRange = keyReader.peekBytes();
    if (keyRange.size() != keyLen) {
      throw std::runtime_error(folly::to<std::string>(
          "Key string of length ",
          keyRange.size(),
          " differs in length from the size specified: ",
          keyLen));
    }
    folly::ssl::EvpPkeyUniquePtr pkey(EVP_PKEY_new_raw_public_key(
        EVP_PKEY_ED25519, nullptr, keyRange.data(), keyLen));

    // Verify the signature
    try {
      fizz::detail::edVerify(message->coalesce(), signature->coalesce(), pkey);
    } catch (const std::exception&) {
      throw std::runtime_error(folly::to<std::string>(
          "Verification failed: ", detail::getOpenSSLError()));
    }
#endif
  } else {
    // rsa_pss and rsa_pkcs
    throw std::runtime_error(
        folly::to<std::string>("key params not implemented: ", keyParams));
  }
}

EcdsaSigUniquePtr Validator::constructECDSASig(const Buf& signature) {
  EcdsaSigUniquePtr ecdsaSignature(ECDSA_SIG_new());
  if (!ecdsaSignature) {
    throw std::runtime_error("Unable to allocate ecdsaSignature");
  }
  Cursor signatureReader(signature.get());
  Buf rBytes = folly::IOBuf::create(TokenBindingUtils::kP256EcKeySize / 2);
  Buf sBytes = folly::IOBuf::create(TokenBindingUtils::kP256EcKeySize / 2);
  signatureReader.clone(*rBytes, TokenBindingUtils::kP256EcKeySize / 2);
  signatureReader.clone(*sBytes, TokenBindingUtils::kP256EcKeySize / 2);
  auto rRange = rBytes->coalesce();
  auto sRange = sBytes->coalesce();
  BIGNUMUniquePtr r(BN_new());
  BIGNUMUniquePtr s(BN_new());
  if (!BN_bin2bn(
          rRange.data(), TokenBindingUtils::kP256EcKeySize / 2, r.get()) ||
      !BN_bin2bn(
          sRange.data(), TokenBindingUtils::kP256EcKeySize / 2, s.get())) {
    throw std::runtime_error("unable to create bnum");
  }

  // ecdsaSignature will clean up Bignum ptrs,
  // so unique ptr needs to release them to avoid double delete
  if (ECDSA_SIG_set0(ecdsaSignature.get(), r.release(), s.release()) != 1) {
    throw std::runtime_error("unable to set bnum on ecdsa_sig");
  }
  return ecdsaSignature;
}

EcKeyUniquePtr Validator::constructEcKeyFromBuf(const Buf& key) {
  // EC_point_oct2point expects the format to match the one described here:
  // https://tlswg.github.io/tls13-spec/draft-ietf-tls-tls13.html#ecdhe-param
  Buf combinedKey = folly::IOBuf::create(TokenBindingUtils::kP256EcKeySize + 1);
  Appender keyAppender(combinedKey.get(), 20);
  keyAppender.writeBE<uint8_t>(POINT_CONVERSION_UNCOMPRESSED);

  // Key string from the token binding message has key size as the first byte,
  // so we need to retrieve the key without the size byte,
  // and add it to the buf from earlier
  Cursor keyReader(key.get());
  auto keyLen = keyReader.readBE<uint8_t>();
  if (keyLen != TokenBindingUtils::kP256EcKeySize) {
    throw std::runtime_error(
        folly::to<std::string>("incorrect key size: ", keyLen));
  }
  keyAppender.push(keyReader, keyLen);
  auto combinedRange = combinedKey->coalesce();

  auto evpKey =
      fizz::detail::decodeECPublicKey(combinedRange, NID_X9_62_prime256v1);
  EcKeyUniquePtr publicKey(EVP_PKEY_get1_EC_KEY(evpKey.get()));
  if (!publicKey) {
    throw std::runtime_error("Error getting EC_key");
  }
  return publicKey;
}
} // namespace extensions
} // namespace fizz
