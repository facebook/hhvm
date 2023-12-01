<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const string OPENSSL_VERSION_TEXT;
const int OPENSSL_VERSION_NUMBER;

const int OPENSSL_RAW_DATA;
const int OPENSSL_PKCS1_PADDING;
const int OPENSSL_ZERO_PADDING;
const int OPENSSL_SSLV23_PADDING;
const int OPENSSL_NO_PADDING;
const int OPENSSL_PKCS1_OAEP_PADDING;

const int OPENSSL_KEYTYPE_RSA;
const int OPENSSL_KEYTYPE_DSA;
const int OPENSSL_KEYTYPE_DH;
const int OPENSSL_KEYTYPE_EC;

const int OPENSSL_ALGO_DSS1;
const int OPENSSL_ALGO_SHA1;
const int OPENSSL_ALGO_SHA224;
const int OPENSSL_ALGO_SHA256;
const int OPENSSL_ALGO_SHA384;
const int OPENSSL_ALGO_SHA512;
const int OPENSSL_ALGO_RMD160;
const int OPENSSL_ALGO_MD5;
const int OPENSSL_ALGO_MD4;
const int OPENSSL_ALGO_MD2;

const int PKCS7_TEXT;
const int PKCS7_BINARY;
const int PKCS7_NOINTERN;
const int PKCS7_NOVERIFY;
const int PKCS7_NOCHAIN;
const int PKCS7_NOCERTS;
const int PKCS7_NOATTR;
const int PKCS7_DETACHED;
const int PKCS7_NOSIGS;

const int OPENSSL_CIPHER_RC2_40;
const int OPENSSL_CIPHER_RC2_128;
const int OPENSSL_CIPHER_RC2_64;
const int OPENSSL_CIPHER_DES;
const int OPENSSL_CIPHER_3DES;

<<__PHPStdLib>>
function openssl_csr_export_to_file(
  HH\FIXME\MISSING_PARAM_TYPE $csr,
  string $outfilename,
  bool $notext = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_csr_export(
  HH\FIXME\MISSING_PARAM_TYPE $csr,
  inout HH\FIXME\MISSING_PARAM_TYPE $out,
  bool $notext = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_csr_get_public_key(
  HH\FIXME\MISSING_PARAM_TYPE $csr,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_csr_get_subject(
  HH\FIXME\MISSING_PARAM_TYPE $csr,
  bool $use_shortnames = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_csr_new(
  HH\FIXME\MISSING_PARAM_TYPE $dn,
  inout HH\FIXME\MISSING_PARAM_TYPE $privkey,
  HH\FIXME\MISSING_PARAM_TYPE $configargs = null,
  HH\FIXME\MISSING_PARAM_TYPE $extraattribs = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_csr_sign(
  HH\FIXME\MISSING_PARAM_TYPE $csr,
  HH\FIXME\MISSING_PARAM_TYPE $cacert,
  HH\FIXME\MISSING_PARAM_TYPE $priv_key,
  int $days,
  HH\FIXME\MISSING_PARAM_TYPE $configargs = null,
  int $serial = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_error_string(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_open(
  string $sealed_data,
  inout HH\FIXME\MISSING_PARAM_TYPE $open_data,
  string $env_key,
  HH\FIXME\MISSING_PARAM_TYPE $priv_key_id,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkcs12_export_to_file(
  HH\FIXME\MISSING_PARAM_TYPE $x509,
  string $filename,
  HH\FIXME\MISSING_PARAM_TYPE $priv_key,
  string $pass,
  HH\FIXME\MISSING_PARAM_TYPE $args = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkcs12_export(
  HH\FIXME\MISSING_PARAM_TYPE $x509,
  inout HH\FIXME\MISSING_PARAM_TYPE $out,
  HH\FIXME\MISSING_PARAM_TYPE $priv_key,
  string $pass,
  HH\FIXME\MISSING_PARAM_TYPE $args = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkcs12_read(
  string $pkcs12,
  inout HH\FIXME\MISSING_PARAM_TYPE $certs,
  string $pass,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkcs7_decrypt(
  string $infilename,
  string $outfilename,
  HH\FIXME\MISSING_PARAM_TYPE $recipcert,
  HH\FIXME\MISSING_PARAM_TYPE $recipkey = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkcs7_encrypt(
  string $infilename,
  string $outfilename,
  HH\FIXME\MISSING_PARAM_TYPE $recipcerts,
  HH\FIXME\MISSING_PARAM_TYPE $headers,
  int $flags = 0,
  int $cipherid = OPENSSL_CIPHER_RC2_40,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkcs7_sign(
  string $infilename,
  string $outfilename,
  HH\FIXME\MISSING_PARAM_TYPE $signcert,
  HH\FIXME\MISSING_PARAM_TYPE $privkey,
  HH\FIXME\MISSING_PARAM_TYPE $headers,
  int $flags = PKCS7_DETACHED,
  string $extracerts = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkcs7_verify(
  string $filename,
  int $flags,
  HH\FIXME\MISSING_PARAM_TYPE $outfilename = null,
  HH\FIXME\MISSING_PARAM_TYPE $cainfo = null,
  HH\FIXME\MISSING_PARAM_TYPE $extracerts = null,
  HH\FIXME\MISSING_PARAM_TYPE $content = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkey_export_to_file(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  string $outfilename,
  string $passphrase = "",
  HH\FIXME\MISSING_PARAM_TYPE $configargs = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkey_export(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  inout HH\FIXME\MISSING_PARAM_TYPE $out,
  string $passphrase = "",
  HH\FIXME\MISSING_PARAM_TYPE $configargs = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkey_free(resource $key): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_free_key(resource $key): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkey_get_details(resource $key): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkey_get_private(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  string $passphrase = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_get_privatekey(
  HH\FIXME\MISSING_PARAM_TYPE $key,
  HH\FIXME\MISSING_PARAM_TYPE $passphrase = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkey_get_public(
  HH\FIXME\MISSING_PARAM_TYPE $certificate,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_get_publickey(
  HH\FIXME\MISSING_PARAM_TYPE $certificate,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_pkey_new(
  HH\FIXME\MISSING_PARAM_TYPE $configargs = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_private_decrypt(
  string $data,
  inout HH\FIXME\MISSING_PARAM_TYPE $decrypted,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  int $padding = OPENSSL_PKCS1_PADDING,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_private_encrypt(
  string $data,
  inout HH\FIXME\MISSING_PARAM_TYPE $crypted,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  int $padding = OPENSSL_PKCS1_PADDING,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_public_decrypt(
  string $data,
  inout HH\FIXME\MISSING_PARAM_TYPE $decrypted,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  int $padding = OPENSSL_PKCS1_PADDING,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_public_encrypt(
  string $data,
  inout HH\FIXME\MISSING_PARAM_TYPE $crypted,
  HH\FIXME\MISSING_PARAM_TYPE $key,
  int $padding = OPENSSL_PKCS1_PADDING,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_seal(
  string $data,
  inout HH\FIXME\MISSING_PARAM_TYPE $sealed_data,
  inout HH\FIXME\MISSING_PARAM_TYPE $env_keys,
  varray<mixed> $pub_key_ids,
  string $method,
  inout HH\FIXME\MISSING_PARAM_TYPE $iv,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_sign(
  string $data,
  inout HH\FIXME\MISSING_PARAM_TYPE $signature,
  HH\FIXME\MISSING_PARAM_TYPE $priv_key_id,
  HH\FIXME\MISSING_PARAM_TYPE $signature_alg = OPENSSL_ALGO_SHA1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_verify(
  string $data,
  string $signature,
  HH\FIXME\MISSING_PARAM_TYPE $pub_key_id,
  HH\FIXME\MISSING_PARAM_TYPE $signature_alg = OPENSSL_ALGO_SHA1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_x509_check_private_key(
  HH\FIXME\MISSING_PARAM_TYPE $cert,
  HH\FIXME\MISSING_PARAM_TYPE $key,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_x509_checkpurpose(
  HH\FIXME\MISSING_PARAM_TYPE $x509cert,
  int $purpose,
  varray<mixed> $cainfo = vec[],
  string $untrustedfile = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_x509_export_to_file(
  HH\FIXME\MISSING_PARAM_TYPE $x509,
  string $outfilename,
  bool $notext = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_x509_export(
  HH\FIXME\MISSING_PARAM_TYPE $x509,
  inout HH\FIXME\MISSING_PARAM_TYPE $output,
  bool $notext = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_x509_free(resource $x509cert): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_x509_parse(
  HH\FIXME\MISSING_PARAM_TYPE $x509cert,
  bool $shortnames = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_x509_read(
  HH\FIXME\MISSING_PARAM_TYPE $x509certdata,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_random_pseudo_bytes(
  int $length,
  inout HH\FIXME\MISSING_PARAM_TYPE $crypto_strong,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_cipher_iv_length(string $method): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_encrypt(
  string $data,
  string $method,
  string $password,
  int $options = 0,
  string $iv = "",
  string $aad = "",
  int $tag_length = 16,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_encrypt_with_tag(
  string $data,
  string $method,
  string $password,
  int $options,
  string $iv,
  inout HH\FIXME\MISSING_PARAM_TYPE $tag_out,
  string $aad = "",
  int $tag_length = 16,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_decrypt(
  string $data,
  string $method,
  string $password,
  int $options = 0,
  string $iv = "",
  string $tag = "",
  string $aad = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_digest(
  string $data,
  string $method,
  bool $raw_output = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_get_cipher_methods(
  bool $aliases = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_get_curve_names(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openssl_get_md_methods(
  bool $aliases = false,
): HH\FIXME\MISSING_RETURN_TYPE;
