#pragma once

#include <sodium.h>

#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/exchange/KeyExchange.h>

#include <fizz/backend/libsodium/crypto/exchange/X25519.h>

namespace fizz::libsodium {

template <class T>
std::unique_ptr<KeyExchange> makeKeyExchange();

template <>
inline std::unique_ptr<KeyExchange> makeKeyExchange<X25519>() {
  return std::make_unique<X25519KeyExchange>();
}

inline void random(unsigned char* out, size_t len) {
  return randombytes_buf(out, len);
}

} // namespace fizz::libsodium
