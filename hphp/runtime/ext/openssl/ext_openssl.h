/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <folly/portability/OpenSSL.h>

#include "hphp/runtime/ext/extension.h"

#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <openssl/pkcs7.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// bitfields
extern const int64_t k_OPENSSL_RAW_DATA;
extern const int64_t k_OPENSSL_ZERO_PADDING;
extern const int64_t k_OPENSSL_NO_PADDING;
extern const int64_t k_OPENSSL_PKCS1_OAEP_PADDING;

// exported constants
extern const int64_t k_OPENSSL_SSLV23_PADDING;
extern const int64_t k_OPENSSL_PKCS1_PADDING;

#define OPENSSL_ALGO_SHA1       1
#define OPENSSL_ALGO_MD5        2
#define OPENSSL_ALGO_MD4        3
#ifdef HAVE_OPENSSL_MD2_H
#define OPENSSL_ALGO_MD2        4
#endif
#define OPENSSL_ALGO_DSS1       5
#if OPENSSL_VERSION_NUMBER >= 0x0090708fL
#define OPENSSL_ALGO_SHA224     6
#define OPENSSL_ALGO_SHA256     7
#define OPENSSL_ALGO_SHA384     8
#define OPENSSL_ALGO_SHA512     9
#define OPENSSL_ALGO_RMD160     10
#endif

#if !defined(OPENSSL_NO_EC) && defined(EVP_PKEY_EC)
#define HAVE_EVP_PKEY_EC 1
#endif

enum php_openssl_key_type {
  OPENSSL_KEYTYPE_RSA,
  OPENSSL_KEYTYPE_DSA,
  OPENSSL_KEYTYPE_DH,
  OPENSSL_KEYTYPE_DEFAULT = OPENSSL_KEYTYPE_RSA,
#ifdef HAVE_EVP_PKEY_EC
  OPENSSL_KEYTYPE_EC = OPENSSL_KEYTYPE_DH + 1
#endif
};

enum php_openssl_cipher_type {
  PHP_OPENSSL_CIPHER_RC2_40,
  PHP_OPENSSL_CIPHER_RC2_128,
  PHP_OPENSSL_CIPHER_RC2_64,
  PHP_OPENSSL_CIPHER_DES,
  PHP_OPENSSL_CIPHER_3DES,
  PHP_OPENSSL_CIPHER_DEFAULT = PHP_OPENSSL_CIPHER_RC2_40
};

Variant openssl_pkcs7_verify_core(const String& filename, int flags,
                                const Variant& voutfilename /* = null_string */,
                                const Variant& vcainfo /* = null_array */,
                                const Variant& vextracerts /* = null_string */,
                                const Variant& vcontent /* = null_string */,
                                bool ignore_cert_expiration);
Variant HHVM_FUNCTION(openssl_random_pseudo_bytes, int64_t length,
                                        bool& crypto_strong);
Variant HHVM_FUNCTION(openssl_cipher_iv_length, const String& method);
Variant HHVM_FUNCTION(openssl_encrypt, const String& data, const String& method,
                                       const String& password,
                                       int64_t options = 0,
                                       const String& iv = null_string,
                                       const String& aad = null_string,
                                       int64_t tag_length = 16);
Variant HHVM_FUNCTION(openssl_decrypt, const String& data, const String& method,
                                       const String& password,
                                       int64_t options = 0,
                                       const String& iv = null_string,
                                       const String& tag = null_string,
                                       const String& aad = null_string);

///////////////////////////////////////////////////////////////////////////////
}
