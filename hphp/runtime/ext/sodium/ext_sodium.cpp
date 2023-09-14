/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:    |
   | http://www.php.net/license/3_01.txt                           |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to   |
   | license@php.net so we can mail you a copy immediately.        |
   +----------------------------------------------------------------------+
*/
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/vm/native.h"

#include <cstring>
#include <sodium.h>
#include <folly/tracing/StaticTracepoint.h>

#include <limits>

namespace HPHP {

namespace {

const StaticString s_arithmetic_overflow("arithmetic overflow");
const StaticString s_internal_error("internal error");

[[noreturn]]
void throwSodiumException(const String& message) {
  Array params;
  params.append(message);
  throw_object("SodiumException", params, true /* init */);
}

const StaticString s_non_string_inout(
  "inout parameter is not a string"
);

String sodium_separate_string(Variant& string_inout) {
  if (!string_inout.isString()) {
    throwSodiumException(s_non_string_inout);
  }
  auto string = string_inout.toString();
  auto data = string.get();
  if (!data->cowCheck()) {
    FOLLY_SDT(hhvm, hhvm_mut_sodium, data->size());
    return string;
  }
  FOLLY_SDT(hhvm, hhvm_cow_sodium, string.size());
  String copy(string, CopyString);
  string_inout = copy;
  return copy;
}

/* Not just wrapping bin2hex() because sodium_bin2hex() is resistant to timing
 * attacks, while the (faster) standard implementation isn't. */
String HHVM_FUNCTION(sodium_bin2hex,
                     const String& binary) {
  if (binary.size() > StringData::MaxSize / 2) {
    throwSodiumException(s_arithmetic_overflow);
  }

  size_t hex_len = binary.size() * 2;
  String hex = String(hex_len, ReserveString);
  sodium_bin2hex(
    hex.mutableData(),
    hex_len + 1,
    reinterpret_cast<const unsigned char*>(binary.data()),
    binary.size()
  );
  hex.setSize(hex_len);

  return hex;
}

/* Not just wrapping hex2bin() because sodium_bin2hex()
 *  - is resistant to timing attacks, while the (faster) standard implementation
 *    isn't
 *  - supports ignoring certain characters - eg for ipv6:
 *    `sodium_hex2bin('[::1]', '[:]')` === 1`
 */
String HHVM_FUNCTION(sodium_hex2bin,
                     const String& hex,
                     const Variant& /* ?string */ ignore) {
  const size_t hex_len = hex.size();
  const size_t bin_len = hex_len / 2;
  String binary(bin_len, ReserveString);

  const char* ignore_chars = nullptr;
  if (ignore.isString()) {
    ignore_chars = ignore.asCStrRef().data();
  };

  size_t real_bin_len;

  auto result = sodium_hex2bin(
    reinterpret_cast<unsigned char*>(binary.mutableData()),
    bin_len,
    hex.data(),
    hex_len,
    ignore_chars,
    &real_bin_len,
    nullptr
  );
  if (result != 0 || real_bin_len > bin_len) {
    throwSodiumException(s_arithmetic_overflow);
  }
  binary.setSize(real_bin_len);

  return binary;
}

const StaticString s_memzero_needs_string("memzero: a PHP string is required");

void HHVM_FUNCTION(sodium_memzero, Variant& buffer) {
  if (!buffer.isString()) {
    throwSodiumException(s_memzero_needs_string);
  }
  auto data = buffer.getStringData();
  /* /// Single ref ///
   * $x = 'foo';
   * sodium_memzero($x);
   * // $x is now null, and the memory was securely wiped
   *
   * /// Multiple refs ///
   * $x = 'foo';
   * $y = $x;
   * sodium_memzero($x);
   * // $x is now null; not securely wiped as that would change the value of $y
   * sodium_memzero($y);
   * // $y is now null, and the memory used by $y's stringdata (and previously
   * $x) is now wiped.
   */
  if (data->hasExactlyOneRef() && !data->empty()) {
    FOLLY_SDT(hhvm, hhvm_mut_sodium, data->size());
    sodium_memzero(data->mutableData(), data->size());
  }
  buffer = init_null();
}

const StaticString s_increment_needs_string(
  "a PHP string is required"
);

void HHVM_FUNCTION(sodium_increment, Variant& buffer_inout) {
  if (!buffer_inout.isString()) {
    throwSodiumException(s_increment_needs_string);
  }

  String buffer = sodium_separate_string(buffer_inout);

  if (buffer.empty()) {
    return;
  }

  sodium_increment(
    reinterpret_cast<unsigned char*>(buffer.mutableData()),
    buffer.size()
  );
}

const StaticString
  s_add_needs_string("PHP strings are required"),
  s_add_same_lengths("values must have the same length");

void HHVM_FUNCTION(sodium_add,
                   Variant& value_inout,
                   const String& add) {
  if (!value_inout.isString()) {
    throwSodiumException(s_add_needs_string);
  }

  String value = sodium_separate_string(value_inout);
  if (value.size() != add.size()) {
    throwSodiumException(s_add_same_lengths);
  }

  sodium_add(
    reinterpret_cast<unsigned char*>(value.mutableData()),
    reinterpret_cast<const unsigned char*>(add.data()),
    value.size()
  );
}

const StaticString s_memcmp_argument_sizes(
  "arguments have different sizes"
);

int64_t HHVM_FUNCTION(sodium_memcmp,
                      const String& a,
                      const String& b) {
  if (a.size() != b.size()) {
    throwSodiumException(s_memcmp_argument_sizes);
  }
  return sodium_memcmp(a.data(), b.data(), a.size());
}

const StaticString s_compare_argument_sizes(
  "arguments have different sizes"
);

int64_t HHVM_FUNCTION(sodium_compare,
                      const String& a,
                      const String& b) {
  if (a.size() != b.size()) {
    throwSodiumException(s_compare_argument_sizes);
  }

  return sodium_compare(
    reinterpret_cast<const unsigned char*>(a.data()),
    reinterpret_cast<const unsigned char*>(b.data()),
    a.size()
  );
}

const StaticString s_crypto_scalarmult_size(
  "scalar and point must be CRYPTO_SCALARMULT_SCALARBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_scalarmult,
                     const String& n,
                     const String& p) {
  if (
    n.size() != crypto_scalarmult_SCALARBYTES ||
    p.size() != crypto_scalarmult_BYTES
  ) {
    throwSodiumException(s_crypto_scalarmult_size);
  }

  String q(crypto_scalarmult_BYTES, ReserveString);
  const auto result = crypto_scalarmult(
    reinterpret_cast<unsigned char*>(q.mutableData()),
    reinterpret_cast<const unsigned char*>(n.data()),
    reinterpret_cast<const unsigned char*>(p.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  q.setSize(crypto_scalarmult_BYTES);
  return q;
}

const StaticString
  s_crypto_generichash_unsupported_output_length(
    "unsupported output length"
  ),
  s_crypto_generichash_unsupported_key_len(
    "unsupported key length"
  );

String HHVM_FUNCTION(sodium_crypto_generichash,
                     const String& message,
                     const Variant& /* ?string */ key,
                     const Variant& /* ?int */ length) {
  size_t hash_len(crypto_generichash_BYTES);
  if (length.isInteger()) {
    hash_len = length.asInt64Val();
  }

  if (
    hash_len < crypto_generichash_BYTES_MIN ||
    hash_len > crypto_generichash_BYTES_MAX
  ) {
    throwSodiumException(s_crypto_generichash_unsupported_output_length);
  }

  size_t key_len = 0;
  const unsigned char* key_data = nullptr;
  if (key.isString()) {
    key_len = key.asCStrRef().size();
    key_data = reinterpret_cast<const unsigned char*>(key.asCStrRef().data());
  }
  if (
    key_len != 0 && (
      key_len < crypto_generichash_KEYBYTES_MIN ||
      key_len > crypto_generichash_KEYBYTES_MAX
    )
  ) {
    throwSodiumException(s_crypto_generichash_unsupported_key_len);
  }

  String hash = String(hash_len, ReserveString);
  auto result = crypto_generichash(
    reinterpret_cast<unsigned char*>(hash.mutableData()),
    hash_len,
    reinterpret_cast<const unsigned char*>(message.data()),
    message.size(),
    key_data,
    key_len
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  hash.setSize(hash_len);
  return hash;
}

const StaticString
  s_crypto_generichash_init_unsupported_output_length(
    "unsupported output length"
  ),
  s_crypto_generichash_init_unsupported_key_len(
    "unsupported key length"
  );

String HHVM_FUNCTION(sodium_crypto_generichash_init,
                     const Variant& /* ?string */ key,
                     const Variant& /* ?int */ length) {
  size_t hash_len(crypto_generichash_BYTES);
  if (length.isInteger()) {
    hash_len = length.asInt64Val();
  }

  if (
    hash_len < crypto_generichash_BYTES_MIN ||
    hash_len > crypto_generichash_BYTES_MAX
  ) {
    throwSodiumException(s_crypto_generichash_init_unsupported_output_length);
  }

  size_t key_len = 0;
  const unsigned char* key_data = nullptr;
  if (key.isString()) {
    key_len = key.asCStrRef().size();
    key_data = reinterpret_cast<const unsigned char*>(key.asCStrRef().data());
  }
  if (
    key_len != 0 && (
      key_len < crypto_generichash_KEYBYTES_MIN ||
      key_len > crypto_generichash_KEYBYTES_MAX
    )
  ) {
    throwSodiumException(s_crypto_generichash_init_unsupported_key_len);
  }

  crypto_generichash_state state_tmp;
  memset(&state_tmp, 0, sizeof state_tmp);
  auto result = crypto_generichash_init(
    &state_tmp,
    key_data,
    key_len,
    hash_len
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }

  size_t state_len = sizeof(crypto_generichash_state);
  String state(state_len, ReserveString);
  memcpy(state.mutableData(), &state_tmp, state_len);
  state.setSize(state_len);
  sodium_memzero(&state_tmp, state_len);
  return state;
}

const StaticString
  s_crypto_generichash_update_state_string_required(
    "incorrect state type, a string is required"
  ),
  s_crypto_generichash_update_incorrect_state_length(
    "incorrect state length"
  );

bool HHVM_FUNCTION(sodium_crypto_generichash_update,
                   Variant& /* string& */ state_inout,
                   const String& msg) {
  if (!state_inout.isString()) {
    throwSodiumException(s_crypto_generichash_update_state_string_required);
  }

  auto state = sodium_separate_string(state_inout);
  size_t state_len = sizeof(crypto_generichash_state);
  if (state.size() != state_len) {
    throwSodiumException(s_crypto_generichash_update_incorrect_state_length);
  }

  crypto_generichash_state state_tmp;
  SCOPE_EXIT {
    sodium_memzero(&state_tmp, state_len);
  };

  memcpy(&state_tmp, state.data(), state_len);
  auto result = crypto_generichash_update(
    &state_tmp,
    reinterpret_cast<const unsigned char*>(msg.data()),
    msg.size()
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  memcpy(state.mutableData(), &state_tmp, state_len);
  return true;
}

const StaticString
  s_crypto_generichash_final_state_string_required(
    "incorrect state type, a string is required"
  ),
  s_crypto_generichash_final_incorrect_state_length(
    "incorrect state length"
  ),
  s_crypto_generichash_final_unsupported_output_length(
    "unsupported output length"
  );

String HHVM_FUNCTION(sodium_crypto_generichash_final,
                     Variant& /* string& */ state_inout,
                     const Variant& /* ?int */ length) {
  if (!state_inout.isString()) {
    throwSodiumException(s_crypto_generichash_final_state_string_required);
  }

  auto state = sodium_separate_string(state_inout);
  size_t state_len = sizeof(crypto_generichash_state);
  if (state.size() != state_len) {
    throwSodiumException(s_crypto_generichash_final_incorrect_state_length);
  }

  size_t hash_len = crypto_generichash_BYTES;
  if (length.isInteger()) {
    hash_len = length.asInt64Val();
  }
  if (
    hash_len < crypto_generichash_BYTES_MIN ||
    hash_len > crypto_generichash_BYTES_MAX
  ) {
    throwSodiumException(s_crypto_generichash_final_unsupported_output_length);
  }

  String hash(hash_len, ReserveString);
  crypto_generichash_state state_tmp;
  SCOPE_EXIT {
    sodium_memzero(&state_tmp, state_len);
  };
  memcpy(&state_tmp, state.data(), state_len);
  auto result = crypto_generichash_final(
    &state_tmp,
    reinterpret_cast<unsigned char*>(hash.mutableData()),
    hash_len
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  hash.setSize(hash_len);

  sodium_memzero(state.mutableData(), state_len);
  state_inout = init_null();

  return hash;
}

const StaticString
  s_shorthash_key_size(
    "key size should be CRYPTO_SHORTHASH_KEYBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_shorthash,
                     const String& message,
                     const String& key) {
  if (key.size() != crypto_shorthash_KEYBYTES) {
    throwSodiumException(s_shorthash_key_size);
  }
  String hash(crypto_shorthash_BYTES, ReserveString);
  const auto result = crypto_shorthash(
    reinterpret_cast<unsigned char*>(hash.mutableData()),
    reinterpret_cast<const unsigned char*>(message.data()),
    message.size(),
    reinterpret_cast<const unsigned char*>(key.data())
  );
  if (result != 0) {
   throwSodiumException(s_internal_error);
  }
  hash.setSize(crypto_shorthash_BYTES);
  return hash;
}

const StaticString
  s_pwhash_salt_size("salt should be CRYPTO_PWHASH_SALTBYTES bytes");

String HHVM_FUNCTION(sodium_crypto_pwhash,
                     int64_t hash_len,
                     const String& password,
                     const String& salt,
                     int64_t opslimit,
                     int64_t memlimit) {
  if (password.empty()) {
    raise_warning("empty password");
  }
  if (salt.size() != crypto_pwhash_SALTBYTES) {
    throwSodiumException(s_pwhash_salt_size);
  }
  if (opslimit < crypto_pwhash_OPSLIMIT_INTERACTIVE) {
    raise_warning("number of operations for the argon2i function is low");
  }
  if (memlimit < crypto_pwhash_MEMLIMIT_INTERACTIVE) {
    raise_warning("maximum memory for the argon2i function is low");
  }

  String hash(hash_len, ReserveString);
  const auto result = crypto_pwhash(
    reinterpret_cast<unsigned char*>(hash.mutableData()),
    hash_len,
    password.data(),
    password.size(),
    reinterpret_cast<const unsigned char*>(salt.data()),
    opslimit,
    memlimit,
    crypto_pwhash_alg_default()
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  hash.setSize(hash_len);
  return hash;
}

String HHVM_FUNCTION(sodium_crypto_pwhash_str,
                     const String& password,
                     int64_t opslimit,
                     int64_t memlimit) {
  if (password.empty()) {
    raise_warning("empty password");
  }
  if (opslimit < crypto_pwhash_OPSLIMIT_INTERACTIVE) {
    raise_warning("number of operations for the argon2i function is low");
  }
  if (memlimit < crypto_pwhash_MEMLIMIT_INTERACTIVE) {
    raise_warning("maximum memory for the argon2i function is low");
  }

  const size_t hash_len = crypto_pwhash_STRBYTES - 1;
  String hash(hash_len, ReserveString);
  const auto result = crypto_pwhash_str(
    reinterpret_cast<char*>(hash.mutableData()),
    password.data(),
    password.size(),
    opslimit,
    memlimit
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  // This is a printable string: the binary data is already base64-encoded by
  // libsodium
  hash.setSize(strlen(hash.data()));
  return hash;
}

bool HHVM_FUNCTION(sodium_crypto_pwhash_str_verify,
                   const String& hash,
                   const String& password) {
  if (password.empty()) {
    raise_warning("empty password");
  }
  const auto result = crypto_pwhash_str_verify(
    hash.data(),
    password.data(),
    password.size()
  );
  return result == 0;
}

const StaticString
  s_pwhash_scrypt_bad_salt_size(
    "salt should be CRYPTO_PWHASH_SCRYPTSALSA208SHA256_SALTBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_pwhash_scryptsalsa208sha256,
                     int64_t length,
                     const String& password,
                     const String& salt,
                     int64_t opslimit,
                     int64_t memlimit) {
  if (password.empty()) {
    raise_warning("empty password");
  }
  if (salt.size() != crypto_pwhash_scryptsalsa208sha256_SALTBYTES) {
    throwSodiumException(s_pwhash_scrypt_bad_salt_size);
  }
  if (opslimit < crypto_pwhash_scryptsalsa208sha256_opslimit_interactive()) {
    raise_warning("number of operations for the scrypt function is low");
  }
  if (memlimit < crypto_pwhash_scryptsalsa208sha256_memlimit_interactive()) {
    raise_warning("maximum memory for the scrypt function is low");
  }

  String hash(length, ReserveString);
  const auto result = crypto_pwhash_scryptsalsa208sha256(
    reinterpret_cast<unsigned char*>(hash.mutableData()),
    length,
    password.data(),
    password.size(),
    reinterpret_cast<const unsigned char*>(salt.data()),
    opslimit,
    memlimit
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  hash.setSize(length);
  return hash;
}

String HHVM_FUNCTION(sodium_crypto_pwhash_scryptsalsa208sha256_str,
                     const String& password,
                     int64_t opslimit,
                     int64_t memlimit) {
  if (password.empty()) {
    raise_warning("empty password");
  }
  if (opslimit < crypto_pwhash_scryptsalsa208sha256_opslimit_interactive()) {
    raise_warning("number of operations for the scrypt function is low");
  }
  if (memlimit < crypto_pwhash_scryptsalsa208sha256_memlimit_interactive()) {
    raise_warning("maximum memory for the scrypt function is low");
  }

  String hash(crypto_pwhash_scryptsalsa208sha256_STRBYTES - 1, ReserveString);
  const auto result = crypto_pwhash_scryptsalsa208sha256_str(
    hash.mutableData(),
    password.data(),
    password.size(),
    opslimit,
    memlimit
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  hash.setSize(crypto_pwhash_scryptsalsa208sha256_STRBYTES - 1);
  return hash;
}

bool HHVM_FUNCTION(
  sodium_crypto_pwhash_scryptsalsa208sha256_str_verify,
  const String& hash,
  const String& password
) {
  if (password.empty()) {
    raise_warning("empty password");
  }
  if (hash.size() != crypto_pwhash_scryptsalsa208sha256_STRBYTES - 1) {
    raise_warning("wrong size for the hashed password");
    return false;
  }

  const auto result = crypto_pwhash_scryptsalsa208sha256_str_verify(
    hash.data(),
    password.data(),
    password.size()
  );
  return result == 0;
}

const StaticString
  s_crypto_auth_key_size(
   "key must be CRYPTO_AUTH_KEYBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_auth,
                    const String& message,
                    const String& key) {
  if (key.size() != crypto_auth_KEYBYTES) {
   throwSodiumException(s_crypto_auth_key_size);
  }
  String mac(crypto_auth_BYTES, ReserveString);
  const auto result = crypto_auth(
   reinterpret_cast<unsigned char*>(mac.mutableData()),
   reinterpret_cast<const unsigned char*>(message.data()),
   message.size(),
   reinterpret_cast<const unsigned char*>(key.data())
  );
  if (result != 0) {
   throwSodiumException(s_internal_error);
  }
  mac.setSize(crypto_auth_BYTES);
  return mac;
}

const StaticString
  s_crypto_auth_verify_key_size(
   "key must be CRYPTO_AUTH_KEYBYTES bytes"
  ),
  s_crypto_auth_mac_size(
   "authentication tag must be CRYPTO_AUTH_BYTES bytes"
  );

bool HHVM_FUNCTION(sodium_crypto_auth_verify,
                   const String& mac,
                   const String& message,
                   const String& key) {
  if (key.size() != crypto_auth_KEYBYTES) {
   throwSodiumException(s_crypto_auth_verify_key_size);
  }
  if (mac.size() != crypto_auth_BYTES) {
   throwSodiumException(s_crypto_auth_mac_size);
  }
  const auto result = crypto_auth_verify(
   reinterpret_cast<const unsigned char*>(mac.data()),
   reinterpret_cast<const unsigned char*>(message.data()),
   message.size(),
   reinterpret_cast<const unsigned char*>(key.data())
  );
  return (result == 0);
}

const StaticString
  s_crypto_secretbox_nonce_size(
    "nonce size should be CRYPTO_SECRETBOX_NONCEBYTES bytes"
  ),
  s_crypto_secretbox_key_size(
    "key size should be CRYPTO_SECRETBOX_KEYBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_secretbox,
                     const String& plaintext,
                     const String& nonce,
                     const String& key) {
  if (nonce.size() != crypto_secretbox_NONCEBYTES) {
    throwSodiumException(s_crypto_secretbox_nonce_size);
  }
  if (key.size() != crypto_secretbox_KEYBYTES) {
    throwSodiumException(s_crypto_secretbox_key_size);
  }
  if (
    StringData::MaxSize - plaintext.size()
      <= crypto_secretbox_MACBYTES
  ) {
    throwSodiumException(s_arithmetic_overflow);
  }

  const size_t ciphertext_len = plaintext.size() + crypto_secretbox_MACBYTES;
  String ciphertext(ciphertext_len, ReserveString);
  const auto result = crypto_secretbox_easy(
    reinterpret_cast<unsigned char*>(ciphertext.mutableData()),
    reinterpret_cast<const unsigned char*>(plaintext.data()),
    plaintext.size(),
    reinterpret_cast<const unsigned char*>(nonce.data()),
    reinterpret_cast<const unsigned char*>(key.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  ciphertext.setSize(ciphertext_len);
  return ciphertext;
}

const StaticString
  s_crypto_secretbox_open_nonce_size(
    "nonce size should be CRYPTO_SECRETBOX_NONCEBYTES "
    "bytes"
  ),
  s_crypto_secretbox_open_key_size(
    "key size should be CRYPTO_SECRETBOX_KEYBYTES "
    "bytes"
  );

Variant HHVM_FUNCTION(sodium_crypto_secretbox_open,
                      const String& ciphertext,
                      const String& nonce,
                      const String& key) {
  if (nonce.size() != crypto_secretbox_NONCEBYTES) {
    throwSodiumException(s_crypto_secretbox_open_nonce_size);
  }
  if (key.size() != crypto_secretbox_KEYBYTES) {
    throwSodiumException(s_crypto_secretbox_open_key_size);
  }
  if (ciphertext.size() < crypto_secretbox_MACBYTES) {
    return false;
  }

  size_t plaintext_len = ciphertext.size() - crypto_secretbox_MACBYTES;
  String plaintext(plaintext_len, ReserveString);
  const auto result = crypto_secretbox_open_easy(
    reinterpret_cast<unsigned char*>(plaintext.mutableData()),
    reinterpret_cast<const unsigned char*>(ciphertext.data()),
    ciphertext.size(),
    reinterpret_cast<const unsigned char*>(nonce.data()),
    reinterpret_cast<const unsigned char*>(key.data())
  );
  if (result != 0) {
    return false;
  }
  plaintext.setSize(plaintext_len);
  return plaintext;
}

String HHVM_FUNCTION(sodium_crypto_box_keypair) {
  // Doing this construction to avoid leaving the secret key in memory
  // somewhere when using temporaries
  const size_t keypair_len =
    crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES;
  String keypair(keypair_len, ReserveString);
  auto secret_key = reinterpret_cast<unsigned char*>(
    keypair.mutableData()
  );
  auto public_key = secret_key + crypto_box_SECRETKEYBYTES;

  const auto result = crypto_box_keypair(
    public_key,
    secret_key
  );

  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  keypair.setSize(keypair_len);
  return keypair;
}

const StaticString
  s_crypto_box_seed_keypair_seed_size(
    "seed should be CRYPTO_BOX_SEEDBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_box_seed_keypair,
                     const String& seed) {
  if (seed.size() != crypto_box_SEEDBYTES) {
    throwSodiumException(s_crypto_box_seed_keypair_seed_size);
  }

  // Doing this construction to avoid leaving the secret key in memory
  // somewhere when using temporaries
  const size_t keypair_len =
   crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES;
  String keypair(keypair_len, ReserveString);
  unsigned char* public_key = reinterpret_cast<unsigned char*>(
    keypair.mutableData() + crypto_box_SECRETKEYBYTES
  );
  unsigned char* secret_key = reinterpret_cast<unsigned char*>(
    keypair.mutableData()
  );

  const auto result = crypto_box_seed_keypair(
    public_key,
    secret_key,
    reinterpret_cast<const unsigned char*>(seed.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  keypair.setSize(keypair_len);
  return keypair;
}

const StaticString s_crypto_box_publickey_from_secretkey_key_size(
  "key should be "
  "CRYPTO_BOX_SECRETKEYBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_box_publickey_from_secretkey,
                     const String& secretkey) {
  if (secretkey.size() != crypto_box_SECRETKEYBYTES) {
    throwSodiumException(s_crypto_box_publickey_from_secretkey_key_size);
  }

  String publickey(crypto_box_PUBLICKEYBYTES, ReserveString);

  static_assert(crypto_scalarmult_BYTES == crypto_box_PUBLICKEYBYTES, "");
  static_assert(crypto_scalarmult_SCALARBYTES == crypto_box_SECRETKEYBYTES, "");
  crypto_scalarmult_base(
    reinterpret_cast<unsigned char*>(publickey.mutableData()),
    reinterpret_cast<const unsigned char*>(secretkey.data())
  );

  publickey.setSize(crypto_box_PUBLICKEYBYTES);
  return publickey;
}

String HHVM_FUNCTION(sodium_crypto_kx_keypair) {
  String keypair(crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES,
                 ReserveString);
  unsigned char* sk = reinterpret_cast<unsigned char*>(keypair.mutableData());
  unsigned char* pk = sk + crypto_kx_SECRETKEYBYTES;
  const auto result = crypto_kx_keypair(pk, sk);
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  keypair.setSize(crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES);
  return keypair;
}

const StaticString s_kx_seed_bad_size(
  "seed must be CRYPTO_KX_SEEDBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_kx_seed_keypair,
                     const String& seed) {
  if (seed.size() != crypto_kx_SEEDBYTES) {
    throwSodiumException(s_kx_seed_bad_size);
  }

  String keypair(crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES,
                 ReserveString);
  unsigned char* sk = reinterpret_cast<unsigned char*>(keypair.mutableData());
  unsigned char* pk = sk + crypto_kx_SECRETKEYBYTES;
  const auto result = crypto_kx_seed_keypair(
    pk,
    sk,
    reinterpret_cast<const unsigned char*>(seed.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  keypair.setSize(crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES);
  return keypair;
}

const StaticString
  s_kx_keypair_bad_size("keypair must be CRYPTO_KX_KEYPAIRBYTES bytes"),
  s_kx_publickey_bad_size("public keys must be CRYPTO_KX_PUBLICKEYBYTES bytes");

#define DEFINE_KX_SESSION_KEYS_FUNC(SIDE) \
Array HHVM_FUNCTION(sodium_crypto_kx_##SIDE##_session_keys,\
                     const String& keypair,\
                     const String& pubkey) {\
  if (keypair.size() != crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES) {\
    throwSodiumException(s_kx_keypair_bad_size);\
  }\
  if (pubkey.size() != crypto_kx_PUBLICKEYBYTES) {\
    throwSodiumException(s_kx_publickey_bad_size);\
  }\
\
  String rx(crypto_kx_SESSIONKEYBYTES, ReserveString);\
  String tx(crypto_kx_SESSIONKEYBYTES, ReserveString);\
\
  auto sk = reinterpret_cast<const unsigned char*>(keypair.data());\
  auto pk = sk + crypto_kx_SECRETKEYBYTES;\
  const auto result = crypto_kx_##SIDE##_session_keys(\
    reinterpret_cast<unsigned char*>(rx.mutableData()),\
    reinterpret_cast<unsigned char*>(tx.mutableData()),\
    pk,\
    sk,\
    reinterpret_cast<const unsigned char*>(pubkey.data())\
  );\
  if (result != 0) {\
    throwSodiumException(s_internal_error);\
  }\
  rx.setSize(crypto_kx_SESSIONKEYBYTES);\
  tx.setSize(crypto_kx_SESSIONKEYBYTES);\
\
  return make_vec_array(rx, tx);\
}
DEFINE_KX_SESSION_KEYS_FUNC(client);
DEFINE_KX_SESSION_KEYS_FUNC(server);

const StaticString
  s_crypto_box_nonce_size(
    "nonce size should be CRYPTO_BOX_NONCEBYTES bytes"
  ),
  s_crypto_box_keypair_size(
    "keypair size should be CRYPTO_BOX_KEYPAIRBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_box,
                     const String& plaintext,
                     const String& nonce,
                     const String& keypair) {
  if (nonce.size() != crypto_box_NONCEBYTES) {
    throwSodiumException(s_crypto_box_nonce_size);
  }
  if (keypair.size() != crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES) {
    throwSodiumException(s_crypto_box_keypair_size);
  }

  auto secretkey = reinterpret_cast<const unsigned char*>(
    keypair.data()
  );
  auto publickey = secretkey + crypto_box_SECRETKEYBYTES;

  if (
    StringData::MaxSize - plaintext.size() <= crypto_box_MACBYTES
  ) {
    throwSodiumException(s_arithmetic_overflow);
  }

  String ciphertext(plaintext.size() + crypto_box_MACBYTES, ReserveString);
  const auto result = crypto_box_easy(
    reinterpret_cast<unsigned char*>(ciphertext.mutableData()),
    reinterpret_cast<const unsigned char*>(plaintext.data()),
    plaintext.size(),
    reinterpret_cast<const unsigned char*>(nonce.data()),
    publickey,
    secretkey
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  ciphertext.setSize(plaintext.size() + crypto_box_MACBYTES);
  return ciphertext;
}

const StaticString
  s_crypto_box_open_nonce_size(
    "nonce size should be CRYPTO_BOX_NONCEBYTES bytes"
  ),
  s_crypto_box_open_keypair_size(
    "keypair size should be CRYPTO_BOX_KEYPAIRBYTES bytes"
  );

Variant HHVM_FUNCTION(sodium_crypto_box_open,
                     const String& ciphertext,
                     const String& nonce,
                     const String& keypair) {
  if (nonce.size() != crypto_box_NONCEBYTES) {
    throwSodiumException(s_crypto_box_open_nonce_size);
  }
  if (keypair.size() != crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES) {
    throwSodiumException(s_crypto_box_open_keypair_size);
  }
  if (ciphertext.size() < crypto_box_MACBYTES) {
    return false;
  }

  auto secretkey = reinterpret_cast<const unsigned char*>(
    keypair.data()
  );
  auto publickey = reinterpret_cast<const unsigned char*>(
    keypair.data() + crypto_box_SECRETKEYBYTES
  );

  String plaintext(ciphertext.size() - crypto_box_MACBYTES, ReserveString);
  const auto result = crypto_box_open_easy(
    reinterpret_cast<unsigned char*>(plaintext.mutableData()),
    reinterpret_cast<const unsigned char*>(ciphertext.data()),
    ciphertext.size(),
    reinterpret_cast<const unsigned char*>(nonce.data()),
    publickey,
    secretkey
  );
  if (result != 0) {
    return false;
  }
  plaintext.setSize(ciphertext.size() - crypto_box_MACBYTES);
  return plaintext;
}

const StaticString
  s_crypto_box_seal_key_size(
    "public key size should be "
    "CRYPTO_BOX_PUBLICKEYBYTES bytes"
  );
String HHVM_FUNCTION(sodium_crypto_box_seal,
                     const String& plaintext,
                     const String& publickey) {
  if (publickey.size() != crypto_box_PUBLICKEYBYTES) {
    throwSodiumException(s_crypto_box_seal_key_size);
  }
  if (
    StringData::MaxSize - plaintext.size()
      <= crypto_box_SEALBYTES
  ) {
    throwSodiumException(s_arithmetic_overflow);
  }

  const size_t ciphertext_len = plaintext.size() + crypto_box_SEALBYTES;
  String ciphertext(ciphertext_len, ReserveString);
  const auto result = crypto_box_seal(
    reinterpret_cast<unsigned char*>(ciphertext.mutableData()),
    reinterpret_cast<const unsigned char*>(plaintext.data()),
    plaintext.size(),
    reinterpret_cast<const unsigned char*>(publickey.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  ciphertext.setSize(ciphertext_len);
  return ciphertext;
}

const StaticString
  s_crypto_box_seal_open_keypair_size(
    "keypair size should be CRYPTO_BOX_KEYPAIRBYTES "
    "bytes"
  );

Variant HHVM_FUNCTION(sodium_crypto_box_seal_open,
                     const String& ciphertext,
                     const String& keypair) {
  if (keypair.size() != crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES) {
    throwSodiumException(s_crypto_box_seal_open_keypair_size);
  }
  if (ciphertext.size() < crypto_box_SEALBYTES) {
    return false;
  }

  auto secretkey = reinterpret_cast<const unsigned char*>(
    keypair.data()
  );
  auto publickey = reinterpret_cast<const unsigned char*>(
    keypair.data() + crypto_box_SECRETKEYBYTES
  );

  const size_t plaintext_len = ciphertext.size() - crypto_box_SEALBYTES;
  String plaintext(plaintext_len, ReserveString);
  const auto result = crypto_box_seal_open(
    reinterpret_cast<unsigned char*>(plaintext.mutableData()),
    reinterpret_cast<const unsigned char*>(ciphertext.data()),
    ciphertext.size(),
    publickey,
    secretkey
  );
  if (result != 0) {
    return false;
  }
  plaintext.setSize(plaintext_len);
  return plaintext;
}

String HHVM_FUNCTION(sodium_crypto_sign_keypair) {
  // Using this construction to avoid leaving secretkey in memory somewhere from
  // a temporary variable
  const size_t keypair_size =
   crypto_sign_SECRETKEYBYTES + crypto_sign_PUBLICKEYBYTES;
  String keypair(keypair_size, ReserveString);
  auto secretkey = reinterpret_cast<unsigned char*>(keypair.mutableData());
  auto publickey = secretkey + crypto_sign_SECRETKEYBYTES;

  const auto result = crypto_sign_keypair(
    publickey,
    secretkey
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  keypair.setSize(keypair_size);
  return keypair;
}

const StaticString
  s_crypto_sign_seed_keypair_seed_size(
    "seed should be CRYPTO_SIGN_SEEDBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_sign_seed_keypair,
                     const String& seed) {
  if (seed.size() != crypto_sign_SEEDBYTES) {
    throwSodiumException(s_crypto_sign_seed_keypair_seed_size);
  }

  // Using this construction to avoid leaving secretkey in memory somewhere from
  // a temporary variable
  const size_t keypair_size =
    crypto_sign_SECRETKEYBYTES + crypto_sign_PUBLICKEYBYTES;
  String keypair(keypair_size, ReserveString);
  auto secretkey = reinterpret_cast<unsigned char*>(keypair.mutableData());
  auto publickey = secretkey + crypto_sign_SECRETKEYBYTES;

  const auto result = crypto_sign_seed_keypair(
    publickey,
    secretkey,
    reinterpret_cast<const unsigned char*>(seed.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  keypair.setSize(keypair_size);
  return keypair;
}

const StaticString
  s_crypto_sign_publickey_from_secretkey_size(
    "secretkey should be "
    "CRYPTO_SIGN_SECRETKEYBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_sign_publickey_from_secretkey,
                     const String& secretkey) {
  if (secretkey.size() != crypto_sign_SECRETKEYBYTES) {
    throwSodiumException(s_crypto_sign_publickey_from_secretkey_size);
  }

  String publickey(crypto_sign_PUBLICKEYBYTES, ReserveString);
  const auto result = crypto_sign_ed25519_sk_to_pk(
    reinterpret_cast<unsigned char*>(publickey.mutableData()),
    reinterpret_cast<const unsigned char*>(secretkey.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  publickey.setSize(crypto_sign_PUBLICKEYBYTES);
  return publickey;
}

const StaticString
  s_crypto_sign_key_size(
    "secret key size should be CRYPTO_SIGN_SECRETKEYBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_sign,
                     const String& message,
                     const String& secretkey) {
  if (secretkey.size() != crypto_sign_SECRETKEYBYTES) {
    throwSodiumException(s_crypto_sign_key_size);
  }
  if (
    StringData::MaxSize - message.size() <= crypto_sign_BYTES
  ) {
    throwSodiumException(s_arithmetic_overflow);
  }

  const size_t signed_message_buffer_size = message.size() + crypto_sign_BYTES;
  unsigned long long signed_message_len;
  String signed_message(signed_message_buffer_size, ReserveString);
  const auto result = crypto_sign(
    reinterpret_cast<unsigned char*>(signed_message.mutableData()),
    &signed_message_len,
    reinterpret_cast<const unsigned char*>(message.data()),
    message.size(),
    reinterpret_cast<const unsigned char*>(secretkey.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }

  if (
    signed_message_len == 0 ||
    signed_message_len >= StringData::MaxSize ||
    signed_message_len > signed_message_buffer_size
  ) {
    throwSodiumException(s_arithmetic_overflow);
  }
  signed_message.setSize(signed_message_len);
  return signed_message;
}

const StaticString s_crypto_sign_open_key_size(
  "public key size should be CRYPTO_SIGN_PUBLICKEYBYTES "
  "bytes"
);

Variant HHVM_FUNCTION(sodium_crypto_sign_open,
                      const String& signed_message,
                      const String& publickey) {
  if (publickey.size() != crypto_sign_PUBLICKEYBYTES) {
    throwSodiumException(s_crypto_sign_open_key_size);
  }

  String message(signed_message.size(), ReserveString);
  unsigned long long message_len;
  const auto result = crypto_sign_open(
    reinterpret_cast<unsigned char*>(message.mutableData()),
    &message_len,
    reinterpret_cast<const unsigned char*>(signed_message.data()),
    signed_message.size(),
    reinterpret_cast<const unsigned char*>(publickey.data())
  );
  if (result != 0) {
    return false;
  }

  if (
    message_len >= StringData::MaxSize ||
    message_len > signed_message.size()
  ) {
    throwSodiumException(s_arithmetic_overflow);
  }
  message.setSize(message_len);
  return message;
}

const StaticString
  s_crypto_sign_detached_key_size(
    "secret key size should be "
    "CRYPTO_SIGN_SECRETKEYBYTES bytes"
  ),
  s_crypto_sign_detached_signature_size(
    "signature has a bogus size"
  );

String HHVM_FUNCTION(sodium_crypto_sign_detached,
                     const String& message,
                     const String& secretkey) {
  if (secretkey.size() != crypto_sign_SECRETKEYBYTES) {
    throwSodiumException(s_crypto_sign_detached_key_size);
  }
  if (
    StringData::MaxSize - message.size() <= crypto_sign_BYTES
  ) {
    throwSodiumException(s_arithmetic_overflow);
  }

  static_assert(crypto_sign_BYTES < StringData::MaxSize, "");

  String signature(crypto_sign_BYTES, ReserveString);
  signature.setSize(crypto_sign_BYTES);
  bzero(signature.mutableData(), crypto_sign_BYTES);

  unsigned long long signature_len;
  const auto result = crypto_sign_detached(
    reinterpret_cast<unsigned char*>(signature.mutableData()),
    &signature_len,
    reinterpret_cast<const unsigned char*>(message.data()),
    message.size(),
    reinterpret_cast<const unsigned char*>(secretkey.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }

  if (
    signature_len == 0 ||
    signature_len > crypto_sign_BYTES
  ) {
    throwSodiumException(s_crypto_sign_detached_signature_size);
  }

  // Intentionally returning a signatured that is crypto_sign_BYTES long instead
  // of signature_len: this has always been required by
  // sodium_crypto_sign_verify_detached(), so this just avoids the need for
  // end-users to pad with null bytes.

  return signature;
}

const StaticString
  s_crypto_sign_verify_detached_signature_size(
    "signature size should be "
    "CRYPTO_SIGN_BYTES bytes"
  ),
  s_crypto_sign_verify_detached_key_size(
    "public key size should be "
    "CRYPTO_SIGN_PUBLICKEYBYTES bytes"
  );

bool HHVM_FUNCTION(sodium_crypto_sign_verify_detached,
                     const String& signature,
                     const String& message,
                     const String& publickey) {
  if (signature.size() != crypto_sign_BYTES) {
    throwSodiumException(s_crypto_sign_verify_detached_signature_size);
  }
  if (publickey.size() != crypto_sign_PUBLICKEYBYTES) {
    throwSodiumException(s_crypto_sign_verify_detached_key_size);
  }

  const auto result = crypto_sign_verify_detached(
    reinterpret_cast<const unsigned char*>(signature.data()),
    reinterpret_cast<const unsigned char*>(message.data()),
    message.size(),
    reinterpret_cast<const unsigned char*>(publickey.data())
  );
  return result == 0;
}

const StaticString
  s_sign_sk_to_box_sk_key_size(
    "Ed25519 key should be "
    "CRYPTO_SIGN_SECRETKEYBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_sign_ed25519_sk_to_curve25519,
                     const String& eddsakey) {
  if (eddsakey.size() != crypto_sign_SECRETKEYBYTES) {
    throwSodiumException(s_sign_sk_to_box_sk_key_size);
  }

  String ecdhkey(crypto_box_SECRETKEYBYTES, ReserveString);
  const auto result = crypto_sign_ed25519_sk_to_curve25519(
    reinterpret_cast<unsigned char*>(ecdhkey.mutableData()),
    reinterpret_cast<const unsigned char*>(eddsakey.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  ecdhkey.setSize(crypto_box_SECRETKEYBYTES);
  return ecdhkey;
}

const StaticString
  s_sign_pk_to_box_pk_key_size(
    "Ed25519 key should be "
    "CRYPTO_SIGN_PUBLICKEYBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_sign_ed25519_pk_to_curve25519,
                     const String& eddsakey) {
  if (eddsakey.size() != crypto_sign_PUBLICKEYBYTES) {
    throwSodiumException(s_sign_pk_to_box_pk_key_size);
  }

  String ecdhkey(crypto_box_PUBLICKEYBYTES, ReserveString);
  const auto result = crypto_sign_ed25519_pk_to_curve25519(
    reinterpret_cast<unsigned char*>(ecdhkey.mutableData()),
    reinterpret_cast<const unsigned char*>(eddsakey.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  ecdhkey.setSize(crypto_box_PUBLICKEYBYTES);
  return ecdhkey;
}

const StaticString
  s_crypto_stream_bad_length("invalid length"),
  s_crypto_stream_nonce_size(
    "nonce should be CRYPTO_STREAM_NONCEBYTES bytes"
  ),
  s_crypto_stream_key_size(
    "key should be CRYPTO_STREAM_KEYEBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_stream,
                     int64_t length,
                     const String& nonce,
                     const String& key) {
  if (length <= 0 || length >= StringData::MaxSize) {
    throwSodiumException(s_crypto_stream_bad_length);
  }
  if (nonce.size() != crypto_stream_NONCEBYTES) {
    throwSodiumException(s_crypto_stream_nonce_size);
  }
  if (key.size() != crypto_stream_KEYBYTES) {
    throwSodiumException(s_crypto_stream_key_size);
  }

  String ciphertext(length, ReserveString);
  const auto result = crypto_stream(
    reinterpret_cast<unsigned char*>(ciphertext.mutableData()),
    length,
    reinterpret_cast<const unsigned char*>(nonce.data()),
    reinterpret_cast<const unsigned char*>(key.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  ciphertext.setSize(length);
  return ciphertext;
}

const StaticString
  s_crypto_stream_xor_nonce_size(
    "nonce should be CRYPTO_STREAM_NONCEBYTES bytes"
  ),
  s_crypto_stream_xor_key_size(
    "key should be CRYPTO_STREAM_KEYEBYTES bytes"
  );

String HHVM_FUNCTION(sodium_crypto_stream_xor,
                     const String& message,
                     const String& nonce,
                     const String& key) {
  if (nonce.size() != crypto_stream_NONCEBYTES) {
    throwSodiumException(s_crypto_stream_xor_nonce_size);
  }
  if (key.size() != crypto_stream_KEYBYTES) {
    throwSodiumException(s_crypto_stream_xor_key_size);
  }

  String ciphertext(message.size(), ReserveString);
  const auto result = crypto_stream_xor(
    reinterpret_cast<unsigned char*>(ciphertext.mutableData()),
    reinterpret_cast<const unsigned char*>(message.data()),
    message.size(),
    reinterpret_cast<const unsigned char*>(nonce.data()),
    reinterpret_cast<const unsigned char*>(key.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  ciphertext.setSize(message.size());
  return ciphertext;
}

const StaticString s_crypto_core_ristretto255_from_hash(
  "scalar must be CRYPTO_CORE_RISTRETTO255_HASHBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_core_ristretto255_from_hash,
                     const String& r) {
  if (
    r.size() != crypto_core_ristretto255_HASHBYTES
  ) {
    throwSodiumException(s_crypto_core_ristretto255_from_hash);
  }

  String p(crypto_core_ristretto255_BYTES, ReserveString);
  crypto_core_ristretto255_from_hash(
    reinterpret_cast<unsigned char*>(p.mutableData()),
    reinterpret_cast<const unsigned char*>(r.data())
  );
  p.setSize(crypto_core_ristretto255_BYTES);
  return p;
}

const StaticString s_crypto_scalarmult_ristretto255(
  "scalar and point must be CRYPTO_SCALARMULT_RISTRETTO255_SCALARBYTES bytes"
);
const StaticString s_crypto_scalarmult_ristretto255_fail(
  "sodium_crypto_scalarmult_ristretto255 failed"
);

String HHVM_FUNCTION(sodium_crypto_scalarmult_ristretto255,
                     const String& n,
                     const String& p) {
  if (
    n.size() != crypto_scalarmult_ristretto255_SCALARBYTES ||
    p.size() != crypto_scalarmult_ristretto255_BYTES
  ) {
    throwSodiumException(s_crypto_scalarmult_ristretto255);
  }

  String q(crypto_scalarmult_ristretto255_BYTES, ReserveString);
  int ret = crypto_scalarmult_ristretto255(
    reinterpret_cast<unsigned char*>(q.mutableData()),
    reinterpret_cast<const unsigned char*>(n.data()),
    reinterpret_cast<const unsigned char*>(p.data())
  );
  if (ret != 0) {
    throwSodiumException(s_crypto_scalarmult_ristretto255_fail);
  }
  q.setSize(crypto_scalarmult_ristretto255_BYTES);
  return q;
}

const StaticString s_crypto_core_ristretto255_scalar_reduce(
  "scalar must be CRYPTO_CORE_RISTRETTO255_NONREDUCEDSCALARBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_core_ristretto255_scalar_reduce,
                     const String& s) {
  if (
    s.size() != crypto_core_ristretto255_NONREDUCEDSCALARBYTES
  ) {
    throwSodiumException(s_crypto_core_ristretto255_scalar_reduce);
  }

  String r(crypto_core_ristretto255_SCALARBYTES, ReserveString);
  crypto_core_ristretto255_scalar_reduce(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(s.data())
  );
  r.setSize(crypto_core_ristretto255_BYTES);
  return r;
}

const StaticString s_crypto_core_ristretto255_scalar_invert(
  "scalar must be CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_core_ristretto255_scalar_invert,
                     const String& s) {
  if (
    s.size() != crypto_core_ristretto255_SCALARBYTES
  ) {
    throwSodiumException(s_crypto_core_ristretto255_scalar_invert);
  }

  String r(crypto_core_ristretto255_SCALARBYTES, ReserveString);
  const auto result = crypto_core_ristretto255_scalar_invert(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(s.data())
  );
  if (result != 0) {
    throwSodiumException(s_internal_error);
  }
  r.setSize(crypto_core_ristretto255_SCALARBYTES);
  return r;
}

const StaticString s_crypto_core_ristretto255_scalar_negate(
  "scalar must be CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_core_ristretto255_scalar_negate,
                     const String& s) {
  if (
    s.size() != crypto_core_ristretto255_SCALARBYTES
  ) {
    throwSodiumException(s_crypto_core_ristretto255_scalar_negate);
  }

  String r(crypto_core_ristretto255_SCALARBYTES, ReserveString);
  crypto_core_ristretto255_scalar_negate(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(s.data())
  );
  r.setSize(crypto_core_ristretto255_SCALARBYTES);
  return r;
}

const StaticString s_crypto_core_ristretto255_scalar_complement(
  "scalar must be CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_core_ristretto255_scalar_complement,
                     const String& s) {
  if (
    s.size() != crypto_core_ristretto255_SCALARBYTES
  ) {
    throwSodiumException(s_crypto_core_ristretto255_scalar_complement);
  }

  String r(crypto_core_ristretto255_SCALARBYTES, ReserveString);
  crypto_core_ristretto255_scalar_complement(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(s.data())
  );
  r.setSize(crypto_core_ristretto255_SCALARBYTES);
  return r;
}

const StaticString s_crypto_core_ristretto255_scalar_add(
  "scalars must be CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_core_ristretto255_scalar_add,
                     const String& x,
                     const String& y) {
  if (
    x.size() != crypto_core_ristretto255_SCALARBYTES ||
    y.size() != crypto_core_ristretto255_SCALARBYTES
  ) {
    throwSodiumException(s_crypto_core_ristretto255_scalar_add);
  }

  String r(crypto_core_ristretto255_SCALARBYTES, ReserveString);
  crypto_core_ristretto255_scalar_add(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(x.data()),
    reinterpret_cast<const unsigned char*>(y.data())
  );
  r.setSize(crypto_core_ristretto255_SCALARBYTES);
  return r;
}

const StaticString s_crypto_core_ristretto255_scalar_sub(
  "scalars must be CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_core_ristretto255_scalar_sub,
                     const String& x,
                     const String& y) {
  if (
    x.size() != crypto_core_ristretto255_SCALARBYTES ||
    y.size() != crypto_core_ristretto255_SCALARBYTES
  ) {
    throwSodiumException(s_crypto_core_ristretto255_scalar_sub);
  }

  String r(crypto_core_ristretto255_SCALARBYTES, ReserveString);
  crypto_core_ristretto255_scalar_sub(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(x.data()),
    reinterpret_cast<const unsigned char*>(y.data())
  );
  r.setSize(crypto_core_ristretto255_SCALARBYTES);
  return r;
}

const StaticString s_crypto_core_ristretto255_scalar_mul(
  "scalars must be CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_core_ristretto255_scalar_mul,
                     const String& x,
                     const String& y) {
  if (
    x.size() != crypto_core_ristretto255_SCALARBYTES ||
    y.size() != crypto_core_ristretto255_SCALARBYTES
  ) {
    throwSodiumException(s_crypto_core_ristretto255_scalar_mul);
  }

  String r(crypto_core_ristretto255_SCALARBYTES, ReserveString);
  crypto_core_ristretto255_scalar_mul(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(x.data()),
    reinterpret_cast<const unsigned char*>(y.data())
  );
  r.setSize(crypto_core_ristretto255_SCALARBYTES);
  return r;
}

const StaticString
  s_subkey_too_small(
    "subkey can not be smaller than sodium_crypto_kdf_BYTES_MIN"),
  s_subkey_too_big(
    "subkey can not be larger than sodium_crypto_kdf_BYTES_MAX"),
  s_negative_subkey_id("subkey_id can not be negative"),
  s_kdf_context_bad_size(
    "context should be sodium_crypto_kdf_CONTEXTBYTES bytes"),
  s_kdf_key_bad_size(
    "key should be sodium_crypto_kdf_KEYBYTES bytes");

String HHVM_FUNCTION(sodium_crypto_kdf_derive_from_key,
                     int64_t subkey_len,
                     int64_t subkey_id,
                     const String& context,
                     const String& key) {
  if (subkey_len < crypto_kdf_BYTES_MIN) {
    throwSodiumException(s_subkey_too_small);
  }
  if (subkey_len > crypto_kdf_BYTES_MAX) {
    throwSodiumException(s_subkey_too_big);
  }
  if (subkey_id < 0) {
    throwSodiumException(s_negative_subkey_id);
  }
  if (context.size() != crypto_kdf_CONTEXTBYTES) {
    throwSodiumException(s_kdf_context_bad_size);
  }
  if (key.size() != crypto_kdf_KEYBYTES) {
    throwSodiumException(s_kdf_key_bad_size);
  }

  String subkey(subkey_len, ReserveString);
  const auto result = crypto_kdf_derive_from_key(
    reinterpret_cast<unsigned char*>(subkey.mutableData()),
    subkey_len,
    subkey_id,
    context.data(),
    reinterpret_cast<const unsigned char*>(key.data())
  );
  if (result != 0) {
    sodium_memzero(subkey.mutableData(), subkey_len);
    throwSodiumException(s_internal_error);
  }
  subkey.setSize(subkey_len);
  return subkey;
}

const StaticString
  s_crypto_core_hchacha20_input_size(
    "input must be CRYPTO_CORE_HCHACHA20_INPUTBYTES bytes"),
  s_crypto_core_hchacha20_key_size(
    "key must be CRYPTO_CORE_HCHACHA20_KEYBYTES bytes"),
  s_crypto_core_hchacha20_constant_size(
    "constant must be CRYPTO_CORE_HCHACHA20_CONSTBYTES bytes or null");

String HHVM_FUNCTION(sodium_crypto_core_hchacha20,
                     const String& in,
                     const String& k,
                     const Variant& /* ?string */ c) {
  if (in.size() != crypto_core_hchacha20_INPUTBYTES) {
    throwSodiumException(s_crypto_core_hchacha20_input_size);
  }
  if (k.size() != crypto_core_hchacha20_KEYBYTES) {
    throwSodiumException(s_crypto_core_hchacha20_key_size);
  }

  const unsigned char* c_data = nullptr;
  if (c.isString()) {
    if (c.asCStrRef().size() != crypto_core_hchacha20_CONSTBYTES) {
      throwSodiumException(s_crypto_core_hchacha20_constant_size);
    }
    c_data = reinterpret_cast<const unsigned char*>(c.asCStrRef().data());
  }

  String out(crypto_core_hchacha20_OUTPUTBYTES, ReserveString);
  const auto result = crypto_core_hchacha20(
    reinterpret_cast<unsigned char*>(out.mutableData()),
    reinterpret_cast<const unsigned char*>(in.data()),
    reinterpret_cast<const unsigned char*>(k.data()),
    c_data
  );
  if (result != 0) {
    sodium_memzero(out.mutableData(), crypto_core_hchacha20_OUTPUTBYTES);
    throwSodiumException(s_internal_error);
  }
  out.setSize(crypto_core_hchacha20_OUTPUTBYTES);
  return out;
}

#define HHVM_SODIUM_AEAD_DECRYPT_FUNCTION(lowercase, uppercase) \
const StaticString\
  s_##lowercase##_decrypt_nonce_size(\
    "crypto_aead_"#lowercase"public nonce size should be "\
    "CRYPTO_AEAD_"#uppercase"_NPUBBYTES bytes"\
  ),\
  s_##lowercase##_decrypt_key_size(\
    "crypto_aead_"#lowercase"secret key size should be "\
    "CRYPTO_AEAD_"#uppercase"_KEYBYTES bytes"\
  );\
\
Variant HHVM_FUNCTION(sodium_crypto_aead_##lowercase##_decrypt,\
                      const String& ciphertext,\
                      const String& ad,\
                      const String& pnonce,\
                      const String& secretkey) {\
  if (pnonce.size() != crypto_aead_##lowercase##_NPUBBYTES) {\
    throwSodiumException(s_##lowercase##_decrypt_nonce_size);\
  }\
  if (secretkey.size() != crypto_aead_##lowercase##_KEYBYTES) {\
    throwSodiumException(s_##lowercase##_decrypt_key_size);\
  }\
\
  if (ciphertext.size() < crypto_aead_##lowercase##_ABYTES) {\
    return false;\
  }\
\
  String plaintext(ciphertext.size(), ReserveString);\
  unsigned long long plaintext_len;\
  const auto result = crypto_aead_##lowercase##_decrypt(\
    reinterpret_cast<unsigned char*>(plaintext.mutableData()),\
    &plaintext_len,\
    nullptr, /* secret nonce */ \
    reinterpret_cast<const unsigned char*>(ciphertext.data()),\
    ciphertext.size(),\
    reinterpret_cast<const unsigned char*>(ad.data()),\
    ad.size(),\
    reinterpret_cast<const unsigned char*>(pnonce.data()),\
    reinterpret_cast<const unsigned char*>(secretkey.data())\
  );\
  if (result != 0) {\
    return false;\
  }\
  if (plaintext_len > ciphertext.size()) {\
    throwSodiumException(s_arithmetic_overflow);\
  }\
  plaintext.setSize(plaintext_len);\
  return plaintext;\
}

#define HHVM_SODIUM_AEAD_ENCRYPT_FUNCTION(lowercase, uppercase) \
const StaticString\
  s_##lowercase##_encrypt_nonce_size(\
    "public nonce size should be "\
    "CRYPTO_AEAD_"#uppercase"_NPUBBYTES bytes"\
  ),\
  s_##lowercase##_encrypt_key_size(\
    "secret key size should be "\
    "CRYPTO_AEAD_"#uppercase"_KEYBYTES bytes"\
  );\
\
String HHVM_FUNCTION(sodium_crypto_aead_##lowercase##_encrypt,\
                     const String& plaintext,\
                     const String& ad,\
                     const String& pnonce,\
                     const String& secretkey) {\
  if (pnonce.size() != crypto_aead_##lowercase##_NPUBBYTES) {\
    throwSodiumException(s_##lowercase##_encrypt_nonce_size);\
  }\
  if (secretkey.size() != crypto_aead_##lowercase##_KEYBYTES) {\
    throwSodiumException(s_##lowercase##_encrypt_key_size);\
  }\
  if (\
    StringData::MaxSize - plaintext.size() <= crypto_aead_##lowercase##_ABYTES\
  ) {\
    throwSodiumException(s_arithmetic_overflow);\
  }\
\
  const size_t ciphertext_buf_size =\
   plaintext.size() + crypto_aead_##lowercase##_ABYTES;\
  String ciphertext(ciphertext_buf_size, ReserveString);\
  unsigned long long ciphertext_len;\
  const auto result = crypto_aead_##lowercase##_encrypt(\
    reinterpret_cast<unsigned char*>(ciphertext.mutableData()),\
    &ciphertext_len,\
    reinterpret_cast<const unsigned char*>(plaintext.data()),\
    plaintext.size(),\
    reinterpret_cast<const unsigned char*>(ad.data()),\
    ad.size(),\
    nullptr, /* secret nonce */\
    reinterpret_cast<const unsigned char*>(pnonce.data()),\
    reinterpret_cast<const unsigned char*>(secretkey.data())\
  );\
  if (result != 0) {\
    throwSodiumException(s_internal_error);\
  }\
  if (ciphertext_len == 0 || ciphertext_len > ciphertext_buf_size) {\
    throwSodiumException(s_arithmetic_overflow);\
  }\
  ciphertext.setSize(ciphertext_len);\
  return ciphertext;\
}

HHVM_SODIUM_AEAD_ENCRYPT_FUNCTION(chacha20poly1305, CHACHA20POLY1305);
HHVM_SODIUM_AEAD_DECRYPT_FUNCTION(chacha20poly1305, CHACHA20POLY1305);

bool HHVM_FUNCTION(sodium_crypto_aead_aes256gcm_is_available) {
  return crypto_aead_aes256gcm_is_available();
}
HHVM_SODIUM_AEAD_ENCRYPT_FUNCTION(aes256gcm, AES256GCM);
HHVM_SODIUM_AEAD_DECRYPT_FUNCTION(aes256gcm, AES256GCM);

HHVM_SODIUM_AEAD_ENCRYPT_FUNCTION(chacha20poly1305_ietf, CHACHA20POLY1305_IETF);
HHVM_SODIUM_AEAD_DECRYPT_FUNCTION(chacha20poly1305_ietf, CHACHA20POLY1305_IETF);

HHVM_SODIUM_AEAD_ENCRYPT_FUNCTION(xchacha20poly1305_ietf,
                                  XCHACHA20POLY1305_IETF);
HHVM_SODIUM_AEAD_DECRYPT_FUNCTION(xchacha20poly1305_ietf,
                                  XCHACHA20POLY1305_IETF);

#define HHVM_REGISTER_AEAD_DEFINITIONS(lowercase, uppercase)\
    HHVM_RC_INT(\
      SODIUM_CRYPTO_AEAD_##uppercase##_KEYBYTES,\
      crypto_aead_##lowercase##_KEYBYTES\
    );\
    HHVM_RC_INT(\
      SODIUM_CRYPTO_AEAD_##uppercase##_NSECBYTES,\
      crypto_aead_##lowercase##_NSECBYTES\
    );\
    HHVM_RC_INT(\
      SODIUM_CRYPTO_AEAD_##uppercase##_NPUBBYTES,\
      crypto_aead_##lowercase##_NPUBBYTES\
    );\
    HHVM_RC_INT(\
      SODIUM_CRYPTO_AEAD_##uppercase##_ABYTES,\
      crypto_aead_##lowercase##_ABYTES\
    );\
    HHVM_FE(sodium_crypto_aead_##lowercase##_decrypt);\
    HHVM_FE(sodium_crypto_aead_##lowercase##_encrypt)


const StaticString s_crypto_secretstream_xchacha20poly130_state_string_required(
  "incorrect state type, a string is required"
),
s_crypto_secretstream_xchacha20poly130_incorrect_key_size(
  "key size should be crypto_secretstream_xchacha20poly1305_KEYBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_keygen) {
  String key(crypto_secretstream_xchacha20poly1305_KEYBYTES, ReserveString);
  crypto_secretstream_xchacha20poly1305_keygen(reinterpret_cast<unsigned char*>(key.mutableData()));
  key.setSize(crypto_secretstream_xchacha20poly1305_KEYBYTES);
  return key;
}

Array HHVM_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_init_push,
                    const String& key) {
  // check key size
  if (key.size() != crypto_secretstream_xchacha20poly1305_KEYBYTES) {
    throwSodiumException(s_crypto_secretstream_xchacha20poly130_state_string_required);
  }
  // secret stream init push
  size_t state_len = sizeof(crypto_secretstream_xchacha20poly1305_state);
  String state(state_len, ReserveString);
  String header(crypto_secretstream_xchacha20poly1305_HEADERBYTES, ReserveString);
  crypto_secretstream_xchacha20poly1305_state st;
  crypto_secretstream_xchacha20poly1305_init_push(&st,
                                                  reinterpret_cast<unsigned char*>(header.mutableData()),
                                                  reinterpret_cast<const unsigned char*>(key.data()));
  header.setSize(crypto_secretstream_xchacha20poly1305_HEADERBYTES);
  // copy state struct into String
  memcpy(state.mutableData(), &st, state_len);
  state.setSize(state_len);
  sodium_memzero(&st, state_len);
  return make_vec_array(state, header);
}

String HHVM_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_push,
                     Variant& state_inout,
                     const String& plaintext,
                     const String& ad,
                     int64_t tag) {
  // parse state string into the struct
  if (!state_inout.isString()) {
    throwSodiumException(s_crypto_secretstream_xchacha20poly130_state_string_required);
  }
  auto state = sodium_separate_string(state_inout);
  size_t state_len = sizeof(crypto_secretstream_xchacha20poly1305_state);
  if (state.size() != state_len) {
    throwSodiumException(s_crypto_generichash_update_incorrect_state_length);
  }
  crypto_secretstream_xchacha20poly1305_state st;
  SCOPE_EXIT {
    sodium_memzero(&st, state_len);
  };
  memcpy(&st, state.data(), state_len);
  // secret stream push
  String ciphertext(plaintext.size() + crypto_secretstream_xchacha20poly1305_ABYTES, ReserveString);
  unsigned long long int ciphertext_len = 0;
  crypto_secretstream_xchacha20poly1305_push(&st,
                                             reinterpret_cast<unsigned char*>(ciphertext.mutableData()),
                                             &ciphertext_len,
                                             reinterpret_cast<const unsigned char*>(plaintext.data()),
                                             plaintext.size(),
                                             reinterpret_cast<const unsigned char*>(ad.data()),
                                             ad.size(),
                                             tag);
  ciphertext.setSize(ciphertext_len);
  // copy state back to string
  memcpy(state.mutableData(), &st, state_len);
  return ciphertext;
}

String HHVM_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_init_pull,
                     const String& key,
                     const String& header) {
  // check key size
  if (key.size() != crypto_secretstream_xchacha20poly1305_KEYBYTES) {
    throwSodiumException(s_crypto_secretstream_xchacha20poly130_state_string_required);
  }
  // secret stream init pull
  size_t state_len = sizeof(crypto_secretstream_xchacha20poly1305_state);
  String state(state_len, ReserveString);
  crypto_secretstream_xchacha20poly1305_state st;
  crypto_secretstream_xchacha20poly1305_init_pull(&st,
                                                  reinterpret_cast<const unsigned char*>(header.data()),
                                                  reinterpret_cast<const unsigned char*>(key.data()));
  // copy state struct to String
  memcpy(state.mutableData(), &st, state_len);
  state.setSize(state_len);
  sodium_memzero(&st, state_len);
  return state;
}

Array HHVM_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_pull,
                    Variant& state_inout,
                    const String& ciphertext,
                    const String& ad)
{
  // parse state string into the struct
  if (!state_inout.isString()) {
    throwSodiumException(s_crypto_secretstream_xchacha20poly130_state_string_required);
  }
  auto state = sodium_separate_string(state_inout);
  size_t state_len = sizeof(crypto_secretstream_xchacha20poly1305_state);
  if (state.size() != state_len) {
    throwSodiumException(s_crypto_generichash_update_incorrect_state_length);
  }
  crypto_secretstream_xchacha20poly1305_state st;
  SCOPE_EXIT {
    sodium_memzero(&st, state_len);
  };
  memcpy(&st, state.data(), state_len);
  // secret stream pull
  String plaintext(ciphertext.size(), ReserveString);
  unsigned long long int plaintext_len = 0;
  unsigned char tag;
  int ret = crypto_secretstream_xchacha20poly1305_pull(
              &st,
              reinterpret_cast<unsigned char*>(plaintext.mutableData()),
              &plaintext_len,
              &tag,
              reinterpret_cast<const unsigned char*>(ciphertext.data()),
              ciphertext.size(),
              reinterpret_cast<const unsigned char*>(ad.data()),
              ad.size()
            );
  if (ret != 0) {
    throwSodiumException(s_internal_error);
  }
  plaintext.setSize(plaintext_len);
  // copy state back to string
  memcpy(state.mutableData(), &st, state_len);
  return make_vec_array(plaintext, tag);
}

void HHVM_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_rekey,
                   Variant& state_inout) {
  // parse state string into the struct
  if (!state_inout.isString()) {
    throwSodiumException(s_crypto_secretstream_xchacha20poly130_state_string_required);
  }
  auto state = sodium_separate_string(state_inout);
  size_t state_len = sizeof(crypto_secretstream_xchacha20poly1305_state);
  if (state.size() != state_len) {
    throwSodiumException(s_crypto_generichash_update_incorrect_state_length);
  }
  crypto_secretstream_xchacha20poly1305_state st;
  SCOPE_EXIT {
    sodium_memzero(&st, state_len);
  };
  memcpy(&st, state.data(), state_len);
  // secret stream rekey
  crypto_secretstream_xchacha20poly1305_rekey(&st);
  // copy state back to string
  memcpy(state.mutableData(), &st, state_len);
}

const StaticString s_crypto_core_ed25519_scalar_size(
  "Ed25519 scalars must be SODIUM_CRYPTO_CORE_ED25519_SCALARBYTES bytes in length"
);

static void validateEd25519Size(const String& x) {
  if (
    x.size() != crypto_core_ed25519_BYTES
  ) {
    throwSodiumException(
     "Ed25519 elements must be SODIUM_CRYPTO_CORE_ED25519_BYTES bytes in length"
    );
  }
}

bool HHVM_FUNCTION(sodium_crypto_core_ed25519_is_valid_point,
                     const String& y) {
  validateEd25519Size(y);
  int r = crypto_core_ed25519_is_valid_point(
    reinterpret_cast<const unsigned char*>(y.data()));
  // libsodium docs: "returns 1 on success, and 0 if the checks didn't pass."
  return r == 1;
};

String HHVM_FUNCTION(sodium_crypto_core_ed25519_add,
                     const String& x,
                     const String& y) {
  validateEd25519Size(x);
  validateEd25519Size(y);
  String r(crypto_core_ed25519_BYTES, ReserveString);
  crypto_core_ed25519_add(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(x.data()),
    reinterpret_cast<const unsigned char*>(y.data())
  );
  r.setSize(crypto_core_ed25519_BYTES);
  return r;
}

String HHVM_FUNCTION(sodium_crypto_core_ed25519_sub,
                     const String& x,
                     const String& y) {
  validateEd25519Size(x);
  validateEd25519Size(y);
  String r(crypto_core_ed25519_BYTES, ReserveString);
  crypto_core_ed25519_sub(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(x.data()),
    reinterpret_cast<const unsigned char*>(y.data())
  );
  r.setSize(crypto_core_ed25519_BYTES);
  return r;
}

String HHVM_FUNCTION(sodium_crypto_scalarmult_ed25519_noclamp,
                     const String& n,
                     const String& p) {
  if (n.size() != crypto_core_ed25519_SCALARBYTES) {
    throwSodiumException(s_crypto_core_ed25519_scalar_size);
  }
  validateEd25519Size(p);
  String r(crypto_core_ed25519_BYTES, ReserveString);
  int rc = crypto_scalarmult_ed25519_noclamp(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(n.data()),
    reinterpret_cast<const unsigned char*>(p.data())
  );
  if (rc != 0) {
    throwSodiumException(
      "Invalid Ed25519 point. Points must be in compressed y coordinate form, in the main subgroup, and not have small order."
    );
  }
  r.setSize(crypto_core_ed25519_BYTES);
  return r;
}


String HHVM_FUNCTION(sodium_crypto_core_ed25519_scalar_add,
                     const String& x,
                     const String& y) {

  if (
    x.size() != crypto_core_ed25519_SCALARBYTES ||
    y.size() != crypto_core_ed25519_SCALARBYTES
  ) {
    throwSodiumException(s_crypto_core_ed25519_scalar_size);
  }

  String r(crypto_core_ed25519_SCALARBYTES, ReserveString);
  crypto_core_ed25519_scalar_add(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(x.data()),
    reinterpret_cast<const unsigned char*>(y.data())
  );
  r.setSize(crypto_core_ed25519_SCALARBYTES);
  return r;
}

String HHVM_FUNCTION(sodium_crypto_core_ed25519_scalar_mul,
                     const String& x,
                     const String& y) {

  if (
    x.size() != crypto_core_ed25519_SCALARBYTES ||
    y.size() != crypto_core_ed25519_SCALARBYTES
  ) {
    throwSodiumException(s_crypto_core_ed25519_scalar_size);
  }

  String r(crypto_core_ed25519_SCALARBYTES, ReserveString);
  crypto_core_ed25519_scalar_mul(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(x.data()),
    reinterpret_cast<const unsigned char*>(y.data())
  );
  r.setSize(crypto_core_ed25519_SCALARBYTES);
  return r;
}

const StaticString s_crypto_core_ed25519_scalar_nonreduced_size(
  "non-reduced scalars must be crypto_core_ed25519_NONREDUCEDSCALARBYTES bytes"
);

String HHVM_FUNCTION(sodium_crypto_core_ed25519_scalar_reduce,
                     const String& x) {
  if (x.size() != crypto_core_ed25519_NONREDUCEDSCALARBYTES) {
    throwSodiumException(s_crypto_core_ed25519_scalar_nonreduced_size);
  }
  String r(crypto_core_ed25519_SCALARBYTES, ReserveString);
  crypto_core_ed25519_scalar_reduce(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(x.data())
  );
  r.setSize(crypto_core_ed25519_SCALARBYTES);
  return r;
}

String HHVM_FUNCTION(sodium_crypto_scalarmult_ed25519_base_noclamp,
                     const String& x) {
  if (x.size() != crypto_core_ed25519_SCALARBYTES) {
    throwSodiumException(s_crypto_core_ed25519_scalar_size);
  }
  String r(crypto_core_ed25519_BYTES, ReserveString);
  crypto_scalarmult_ed25519_base_noclamp(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(x.data())
  );
  r.setSize(crypto_core_ed25519_BYTES);
  return r;
}

String HHVM_FUNCTION(sodium_crypto_scalarmult_ed25519_base,
                     const String& x) {
  if (x.size() != crypto_core_ed25519_SCALARBYTES) {
    throwSodiumException(s_crypto_core_ed25519_scalar_size);
  }
  String r(crypto_core_ed25519_BYTES, ReserveString);
  crypto_scalarmult_ed25519_base_noclamp(
    reinterpret_cast<unsigned char*>(r.mutableData()),
    reinterpret_cast<const unsigned char*>(x.data())
  );
  r.setSize(crypto_core_ed25519_BYTES);
  return r;
}

struct SodiumExtension final : Extension {
  SodiumExtension() : Extension("sodium", "7.2-hhvm1", NO_ONCALL_YET) {}

  void moduleInit() override {
    if (sodium_init() == -1) {
      raise_error("sodium_init()");
    }

    HHVM_RC_STR(SODIUM_LIBRARY_VERSION, sodium_version_string());
    HHVM_RC_INT(SODIUM_LIBRARY_MAJOR_VERSION, sodium_library_version_major());
    HHVM_RC_INT(SODIUM_LIBRARY_MINOR_VERSION, sodium_library_version_minor());

    HHVM_FE(sodium_bin2hex);
    HHVM_FE(sodium_hex2bin);
    HHVM_FE(sodium_memzero);
    HHVM_FE(sodium_increment);
    HHVM_FE(sodium_add);
    HHVM_FE(sodium_memcmp);
    HHVM_FE(sodium_compare);

    HHVM_RC_INT(SODIUM_CRYPTO_SCALARMULT_BYTES, crypto_scalarmult_BYTES);
    HHVM_RC_INT(
      SODIUM_CRYPTO_SCALARMULT_SCALARBYTES,
      crypto_scalarmult_SCALARBYTES
    );
    HHVM_FE(sodium_crypto_scalarmult);

    HHVM_RC_INT(SODIUM_CRYPTO_GENERICHASH_KEYBYTES,
                crypto_generichash_KEYBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_GENERICHASH_KEYBYTES_MIN,
                crypto_generichash_KEYBYTES_MIN);
    HHVM_RC_INT(SODIUM_CRYPTO_GENERICHASH_KEYBYTES_MAX,
                crypto_generichash_KEYBYTES_MAX);
    HHVM_FE(sodium_crypto_generichash);
    HHVM_FE(sodium_crypto_generichash_init);
    HHVM_FE(sodium_crypto_generichash_update);
    HHVM_FE(sodium_crypto_generichash_final);

    HHVM_RC_INT(SODIUM_CRYPTO_SHORTHASH_BYTES,crypto_shorthash_BYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_SHORTHASH_KEYBYTES,crypto_shorthash_KEYBYTES);
    HHVM_FE(sodium_crypto_shorthash);

    HHVM_RC_INT(SODIUM_CRYPTO_PWHASH_SALTBYTES, crypto_pwhash_SALTBYTES);
    HHVM_RC_STR(SODIUM_CRYPTO_PWHASH_STRPREFIX, crypto_pwhash_STRPREFIX);
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_OPSLIMIT_INTERACTIVE,
      crypto_pwhash_OPSLIMIT_INTERACTIVE
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_MEMLIMIT_INTERACTIVE,
      crypto_pwhash_MEMLIMIT_INTERACTIVE
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_OPSLIMIT_MODERATE,
      crypto_pwhash_OPSLIMIT_MODERATE
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_MEMLIMIT_MODERATE,
      crypto_pwhash_MEMLIMIT_MODERATE
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_OPSLIMIT_SENSITIVE,
      crypto_pwhash_OPSLIMIT_SENSITIVE
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_MEMLIMIT_SENSITIVE,
      crypto_pwhash_MEMLIMIT_SENSITIVE
    );
    HHVM_FE(sodium_crypto_pwhash);
    HHVM_FE(sodium_crypto_pwhash_str);
    HHVM_FE(sodium_crypto_pwhash_str_verify);

    HHVM_RC_STR(
      SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_STRPREFIX,
      crypto_pwhash_scryptsalsa208sha256_STRPREFIX
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_SALTBYTES,
      crypto_pwhash_scryptsalsa208sha256_SALTBYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_OPSLIMIT_INTERACTIVE,
      (int64_t) crypto_pwhash_scryptsalsa208sha256_opslimit_interactive()
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_MEMLIMIT_INTERACTIVE,
      (int64_t) crypto_pwhash_scryptsalsa208sha256_memlimit_interactive()
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_OPSLIMIT_SENSITIVE,
      (int64_t) crypto_pwhash_scryptsalsa208sha256_opslimit_sensitive()
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_MEMLIMIT_SENSITIVE,
      (int64_t) crypto_pwhash_scryptsalsa208sha256_memlimit_sensitive()
    );
    HHVM_FE(sodium_crypto_pwhash_scryptsalsa208sha256);
    HHVM_FE(sodium_crypto_pwhash_scryptsalsa208sha256_str);
    HHVM_FE(sodium_crypto_pwhash_scryptsalsa208sha256_str_verify);

    HHVM_RC_INT(SODIUM_CRYPTO_AUTH_BYTES, crypto_auth_BYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_AUTH_KEYBYTES, crypto_auth_KEYBYTES);
    HHVM_FE(sodium_crypto_auth);
    HHVM_FE(sodium_crypto_auth_verify);

    HHVM_RC_INT(SODIUM_CRYPTO_SECRETBOX_KEYBYTES, crypto_secretbox_KEYBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_SECRETBOX_MACBYTES, crypto_secretbox_MACBYTES);
    HHVM_RC_INT(
      SODIUM_CRYPTO_SECRETBOX_NONCEBYTES, crypto_secretbox_NONCEBYTES);
    HHVM_FE(sodium_crypto_secretbox);
    HHVM_FE(sodium_crypto_secretbox_open);

    HHVM_RC_INT(SODIUM_CRYPTO_BOX_SECRETKEYBYTES, crypto_box_SECRETKEYBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_BOX_PUBLICKEYBYTES, crypto_box_PUBLICKEYBYTES);
    HHVM_RC_INT(
      SODIUM_CRYPTO_BOX_KEYPAIRBYTES,
      crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES
    );
    HHVM_RC_INT(SODIUM_CRYPTO_BOX_MACBYTES, crypto_box_MACBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_BOX_NONCEBYTES, crypto_box_NONCEBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_BOX_SEEDBYTES, crypto_box_SEEDBYTES);
    HHVM_FE(sodium_crypto_box_keypair);
    HHVM_FE(sodium_crypto_box_seed_keypair);
    HHVM_FE(sodium_crypto_box_publickey_from_secretkey);
    HHVM_FALIAS(
      sodium_crypto_scalarmult_base,
      sodium_crypto_box_publickey_from_secretkey
    );

    HHVM_FE(sodium_crypto_box);
    HHVM_FE(sodium_crypto_box_open);

    HHVM_RC_INT(SODIUM_CRYPTO_BOX_SEALBYTES, crypto_box_SEALBYTES);
    HHVM_FE(sodium_crypto_box_seal);
    HHVM_FE(sodium_crypto_box_seal_open);

    HHVM_RC_INT(SODIUM_CRYPTO_SIGN_BYTES, crypto_sign_BYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_SIGN_SEEDBYTES, crypto_sign_SEEDBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_SIGN_PUBLICKEYBYTES, crypto_sign_PUBLICKEYBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_SIGN_SECRETKEYBYTES, crypto_sign_SECRETKEYBYTES);
    HHVM_RC_INT(
      SODIUM_CRYPTO_SIGN_KEYPAIRBYTES,
      crypto_sign_PUBLICKEYBYTES + crypto_sign_SECRETKEYBYTES
    );

    HHVM_FE(sodium_crypto_sign_keypair);
    HHVM_FE(sodium_crypto_sign_seed_keypair);
    HHVM_FE(sodium_crypto_sign_publickey_from_secretkey);
    HHVM_FE(sodium_crypto_sign_ed25519_pk_to_curve25519);
    HHVM_FE(sodium_crypto_sign_ed25519_sk_to_curve25519);

    HHVM_FE(sodium_crypto_sign);
    HHVM_FE(sodium_crypto_sign_open);
    HHVM_FE(sodium_crypto_sign_detached);
    HHVM_FE(sodium_crypto_sign_verify_detached);

    HHVM_RC_INT(SODIUM_CRYPTO_STREAM_NONCEBYTES, crypto_stream_NONCEBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_STREAM_KEYBYTES, crypto_stream_KEYBYTES);
    HHVM_FE(sodium_crypto_stream);
    HHVM_FE(sodium_crypto_stream_xor);

    HHVM_REGISTER_AEAD_DEFINITIONS(chacha20poly1305, CHACHA20POLY1305);
    HHVM_FE(sodium_crypto_aead_aes256gcm_is_available);
    HHVM_REGISTER_AEAD_DEFINITIONS(aes256gcm, AES256GCM);

    HHVM_REGISTER_AEAD_DEFINITIONS(chacha20poly1305_ietf,
                                   CHACHA20POLY1305_IETF);

    HHVM_REGISTER_AEAD_DEFINITIONS(xchacha20poly1305_ietf,
                                   XCHACHA20POLY1305_IETF);

    HHVM_FE(sodium_crypto_kdf_derive_from_key);
    HHVM_RC_INT(SODIUM_CRYPTO_KDF_BYTES_MIN, crypto_kdf_BYTES_MIN);
    HHVM_RC_INT(SODIUM_CRYPTO_KDF_BYTES_MAX, crypto_kdf_BYTES_MAX);
    HHVM_RC_INT(SODIUM_CRYPTO_KDF_CONTEXTBYTES, crypto_kdf_CONTEXTBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_KDF_KEYBYTES, crypto_kdf_KEYBYTES);

    HHVM_FE(sodium_crypto_core_hchacha20);
    HHVM_RC_INT(SODIUM_CRYPTO_CORE_HCHACHA20_INPUTBYTES, crypto_core_hchacha20_INPUTBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_CORE_HCHACHA20_KEYBYTES, crypto_core_hchacha20_KEYBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_CORE_HCHACHA20_OUTPUTBYTES, crypto_core_hchacha20_OUTPUTBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_CORE_HCHACHA20_CONSTBYTES, crypto_core_hchacha20_CONSTBYTES);

    HHVM_RC_INT(SODIUM_CRYPTO_KX_PUBLICKEYBYTES, crypto_kx_PUBLICKEYBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_KX_SESSIONKEYBYTES, crypto_kx_SESSIONKEYBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_KX_SECRETKEYBYTES, crypto_kx_SECRETKEYBYTES);
    HHVM_RC_INT(SODIUM_CRYPTO_KX_KEYPAIRBYTES,
                crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES);
    HHVM_FE(sodium_crypto_kx_keypair);
    HHVM_FE(sodium_crypto_kx_seed_keypair);
    HHVM_FE(sodium_crypto_kx_client_session_keys);
    HHVM_FE(sodium_crypto_kx_server_session_keys);

    HHVM_RC_INT(
      SODIUM_CRYPTO_SCALARMULT_RISTRETTO255_BYTES,
      crypto_scalarmult_ristretto255_BYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_SCALARMULT_RISTRETTO255_SCALARBYTES,
      crypto_scalarmult_ristretto255_SCALARBYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_CORE_RISTRETTO255_BYTES,
      crypto_core_ristretto255_BYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_CORE_RISTRETTO255_HASHBYTES,
      crypto_core_ristretto255_HASHBYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES,
      crypto_core_ristretto255_SCALARBYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_CORE_RISTRETTO255_NONREDUCEDSCALARBYTES,
      crypto_core_ristretto255_NONREDUCEDSCALARBYTES
    );
    HHVM_FE(sodium_crypto_core_ristretto255_from_hash);
    HHVM_FE(sodium_crypto_scalarmult_ristretto255);
    HHVM_FE(sodium_crypto_core_ristretto255_scalar_reduce);
    HHVM_FE(sodium_crypto_core_ristretto255_scalar_invert);
    HHVM_FE(sodium_crypto_core_ristretto255_scalar_negate);
    HHVM_FE(sodium_crypto_core_ristretto255_scalar_complement);
    HHVM_FE(sodium_crypto_core_ristretto255_scalar_add);
    HHVM_FE(sodium_crypto_core_ristretto255_scalar_sub);
    HHVM_FE(sodium_crypto_core_ristretto255_scalar_mul);

    HHVM_RC_INT(
      SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_ABYTES,
      crypto_secretstream_xchacha20poly1305_ABYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_HEADERBYTES,
      crypto_secretstream_xchacha20poly1305_HEADERBYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_KEYBYTES,
      crypto_secretstream_xchacha20poly1305_KEYBYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_MESSAGEBYTES_MAX,
      crypto_secretstream_xchacha20poly1305_MESSAGEBYTES_MAX
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_TAG_MESSAGE,
      crypto_secretstream_xchacha20poly1305_TAG_MESSAGE
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_TAG_PUSH,
      crypto_secretstream_xchacha20poly1305_TAG_PUSH
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_TAG_REKEY,
      crypto_secretstream_xchacha20poly1305_TAG_REKEY
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_TAG_FINAL,
      crypto_secretstream_xchacha20poly1305_TAG_FINAL
    );
    HHVM_FE(sodium_crypto_secretstream_xchacha20poly1305_keygen);
    HHVM_FE(sodium_crypto_secretstream_xchacha20poly1305_init_push);
    HHVM_FE(sodium_crypto_secretstream_xchacha20poly1305_push);
    HHVM_FE(sodium_crypto_secretstream_xchacha20poly1305_init_pull);
    HHVM_FE(sodium_crypto_secretstream_xchacha20poly1305_pull);
    HHVM_FE(sodium_crypto_secretstream_xchacha20poly1305_rekey);

    HHVM_RC_INT(
      SODIUM_CRYPTO_CORE_ED25519_SCALARBYTES,
      crypto_core_ed25519_SCALARBYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_CORE_ED25519_NONREDUCEDSCALARBYTES,
      crypto_core_ed25519_NONREDUCEDSCALARBYTES
    );
    HHVM_RC_INT(
      SODIUM_CRYPTO_CORE_ED25519_BYTES,
      crypto_core_ed25519_BYTES
    );
    HHVM_FE(sodium_crypto_core_ed25519_is_valid_point);
    HHVM_FE(sodium_crypto_core_ed25519_add);
    HHVM_FE(sodium_crypto_core_ed25519_sub);
    HHVM_FE(sodium_crypto_core_ed25519_scalar_reduce);
    HHVM_FE(sodium_crypto_core_ed25519_scalar_mul);
    HHVM_FE(sodium_crypto_core_ed25519_scalar_add);
    HHVM_FE(sodium_crypto_scalarmult_ed25519_noclamp);
    HHVM_FE(sodium_crypto_scalarmult_ed25519_base);
    HHVM_FE(sodium_crypto_scalarmult_ed25519_base_noclamp);
  }
} s_sodium_extension;

} // namespace

} // namespace HPHP
