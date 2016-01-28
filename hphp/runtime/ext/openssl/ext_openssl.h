/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_OPENSSL_H_
#define incl_HPHP_EXT_OPENSSL_H_

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

enum php_openssl_key_type {
  OPENSSL_KEYTYPE_RSA,
  OPENSSL_KEYTYPE_DSA,
  OPENSSL_KEYTYPE_DH,
  OPENSSL_KEYTYPE_DEFAULT = OPENSSL_KEYTYPE_RSA,
#ifdef EVP_PKEY_EC
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

bool HHVM_FUNCTION(openssl_csr_export_to_file, const Variant& csr,
                                               const String& outfilename,
                                               bool notext = true);
bool HHVM_FUNCTION(openssl_csr_export, const Variant& csr, VRefParam out,
                                       bool notext = true);
Variant HHVM_FUNCTION(openssl_csr_get_public_key, const Variant& csr);
Variant HHVM_FUNCTION(openssl_csr_get_subject, const Variant& csr,
                      bool use_shortnames = true);
Variant HHVM_FUNCTION(openssl_csr_new,
                      const Variant& dn, VRefParam privkey,
                      const Variant& configargs = null_variant,
                      const Variant& extraattribs = null_variant);
Variant HHVM_FUNCTION(openssl_csr_sign,
                      const Variant& csr,
                      const Variant& cacert,
                      const Variant& priv_key, int days,
                      const Variant& configargs = null_variant,
                      int serial = 0);
Variant HHVM_FUNCTION(openssl_error_string);
bool HHVM_FUNCTION(openssl_open, const String& sealed_data, VRefParam open_data,
                                 const String& env_key,
                                 const Variant& priv_key_id,
                                 const String& method = null_string,
                                 const String& iv = null_string);
bool HHVM_FUNCTION(openssl_pkcs12_export_to_file, const Variant& x509,
                                                  const String& filename,
                                                  const Variant& priv_key,
                                                  const String& pass,
                                    const Variant& args = null_variant);
bool HHVM_FUNCTION(openssl_pkcs12_export, const Variant& x509, VRefParam out,
                                          const Variant& priv_key,
                                          const String& pass,
                                    const Variant& args = null_variant);
bool HHVM_FUNCTION(openssl_pkcs12_read, const String& pkcs12, VRefParam certs,
                                        const String& pass);
bool HHVM_FUNCTION(openssl_pkcs7_decrypt, const String& infilename,
                                          const String& outfilename,
                                          const Variant& recipcert,
                                const Variant& recipkey = null_variant);
bool HHVM_FUNCTION(openssl_pkcs7_encrypt, const String& infilename,
                                          const String& outfilename,
                                          const Variant& recipcerts,
                                          const Array& headers,
                                          int flags = 0,
                                int cipherid = PHP_OPENSSL_CIPHER_RC2_40);
bool HHVM_FUNCTION(openssl_pkcs7_sign, const String& infilename,
                                       const String& outfilename,
                                       const Variant& signcert,
                                       const Variant& privkey,
                                       const Variant& headers,
                                       int flags = PKCS7_DETACHED,
                                const String& extracerts = null_string);
Variant openssl_pkcs7_verify_core(const String& filename, int flags,
                                const Variant& voutfilename /* = null_string */,
                                const Variant& vcainfo /* = null_array */,
                                const Variant& vextracerts /* = null_string */,
                                const Variant& vcontent /* = null_string */,
                                bool ignore_cert_expiration);
Variant HHVM_FUNCTION(openssl_pkcs7_verify, const String& filename, int flags,
                               const Variant& outfilename = null_string,
                               const Variant& cainfo = null_array,
                               const Variant& extracerts = null_string,
                               const Variant& content = null_string);
Variant HHVM_FUNCTION(fb_unsafe_openssl_pkcs7_verify_ignore_cert_expiration,
                               const String& filename, int flags,
                               const Variant& outfilename = null_string,
                               const Variant& cainfo = null_array,
                               const Variant& extracerts = null_string,
                               const Variant& content = null_string);
bool HHVM_FUNCTION(openssl_pkey_export_to_file, const Variant& key,
                                                const String& outfilename,
                                   const String& passphrase = null_string,
                               const Variant& configargs = null_variant);
bool HHVM_FUNCTION(openssl_pkey_export, const Variant& key, VRefParam out,
                                   const String& passphrase = null_string,
                              const Variant& configargs = null_variant);
Array HHVM_FUNCTION(openssl_pkey_get_details, const Resource& key);
Variant HHVM_FUNCTION(openssl_pkey_get_private, const Variant& key,
                                 const String& passphrase = null_string);
Variant HHVM_FUNCTION(openssl_pkey_get_public, const Variant& certificate);
Resource HHVM_FUNCTION(openssl_pkey_new,
                       const Variant& configargs = null_variant);
bool HHVM_FUNCTION(openssl_private_decrypt, const String& data,
                                            VRefParam decrypted,
                                            const Variant& key,
                                  int padding = k_OPENSSL_PKCS1_PADDING);
bool HHVM_FUNCTION(openssl_private_encrypt, const String& data,
                                            VRefParam crypted,
                                            const Variant& key,
                                  int padding = k_OPENSSL_PKCS1_PADDING);
bool HHVM_FUNCTION(openssl_public_decrypt, const String& data,
                                           VRefParam decrypted,
                                           const Variant& key,
                                  int padding = k_OPENSSL_PKCS1_PADDING);
bool HHVM_FUNCTION(openssl_public_encrypt, const String& data,
                                           VRefParam crypted,
                                           const Variant& key,
                                  int padding = k_OPENSSL_PKCS1_PADDING);
Variant HHVM_FUNCTION(openssl_seal, const String& data, VRefParam sealed_data,
                                    VRefParam env_keys,
                                    const Array& pub_key_ids,
                                    const String& method = null_string,
                                    VRefParam iv = null_string);
bool HHVM_FUNCTION(openssl_sign, const String& data, VRefParam signature,
                                 const Variant& priv_key_id,
                     const Variant& signature_alg = OPENSSL_ALGO_SHA1);
Variant HHVM_FUNCTION(openssl_verify, const String& data,
                                      const String& signature,
                                      const Variant& pub_key_id,
                     const Variant& signature_alg = OPENSSL_ALGO_SHA1);
bool HHVM_FUNCTION(openssl_x509_check_private_key, const Variant& cert,
                                                   const Variant& key);
Variant HHVM_FUNCTION(openssl_x509_checkpurpose, const Variant& x509cert,
                      int purpose,
                      const Array& cainfo = null_array,
                      const String& untrustedfile = null_string);
bool HHVM_FUNCTION(openssl_x509_export_to_file, const Variant& x509,
                                                const String& outfilename,
                                                bool notext = true);
bool HHVM_FUNCTION(openssl_x509_export, const Variant& x509, VRefParam output,
                                        bool notext = true);
Variant HHVM_FUNCTION(openssl_x509_parse, const Variant& x509cert,
                                          bool shortnames = true);
Variant HHVM_FUNCTION(openssl_x509_read, const Variant& x509certdata);
Variant HHVM_FUNCTION(openssl_random_pseudo_bytes, int length,
                                        VRefParam crypto_strong = false);
Variant HHVM_FUNCTION(openssl_cipher_iv_length, const String& method);
Variant HHVM_FUNCTION(openssl_encrypt, const String& data, const String& method,
                                       const String& password,
                                       int options = 0,
                                       const String& iv = null_string);
Variant HHVM_FUNCTION(openssl_decrypt, const String& data, const String& method,
                                       const String& password,
                                       int options = 0,
                                       const String& iv = null_string);
Variant HHVM_FUNCTION(openssl_digest, const String& data, const String& method,
                                      bool raw_output = false);
Array HHVM_FUNCTION(openssl_get_cipher_methods, bool aliases = false);
Array HHVM_FUNCTION(openssl_get_md_methods, bool aliases = false);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_OPENSSL_H_
