// Copyright 2004-present Facebook. All Rights Reserved.
#pragma once

#include <fizz/fizz-config.h>

#if FIZZ_HAVE_OQS

#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/exchange/KeyExchange.h>

#include <fizz/backend/liboqs/OQSKeyExchange.h>
#include <oqs/kem.h>

namespace fizz::liboqs {

template <class T>
struct Properties;

template <>
struct Properties<fizz::Kyber512> {
  static constexpr const char* id = OQS_KEM_alg_kyber_512;
};

template <>
struct Properties<fizz::Kyber768> {
  static constexpr const char* id = OQS_KEM_alg_kyber_768;
};

template <>
struct Properties<fizz::MLKEM512> {
  static constexpr const char* id = OQS_KEM_alg_ml_kem_512;
};

template <>
struct Properties<fizz::MLKEM768> {
  static constexpr const char* id = OQS_KEM_alg_ml_kem_768;
};

template <class T>
std::unique_ptr<fizz::KeyExchange> makeKeyExchange(KeyExchangeRole role) {
  return OQSKeyExchange::createOQSKeyExchange(role, Properties<T>::id);
}

} // namespace fizz::liboqs

#endif
