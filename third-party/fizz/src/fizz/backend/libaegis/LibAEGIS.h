#pragma once

#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/aead/Aead.h>
#include <fizz/fizz-config.h>

#if FIZZ_HAVE_LIBAEGIS
namespace fizz::libaegis {

template <class T>
std::unique_ptr<Aead> makeCipher();

template <>
std::unique_ptr<Aead> makeCipher<fizz::AEGIS128L>();
template <>
std::unique_ptr<Aead> makeCipher<fizz::AEGIS256>();
} // namespace fizz::libaegis
#endif
