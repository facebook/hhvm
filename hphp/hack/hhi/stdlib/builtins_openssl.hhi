<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const string OPENSSL_VERSION_TEXT = '';
const int OPENSSL_VERSION_NUMBER = 0;

const int OPENSSL_RAW_DATA = 1;
const int OPENSSL_PKCS1_PADDING = 1;
const int OPENSSL_ZERO_PADDING = 2;
const int OPENSSL_SSLV23_PADDING = 2;
const int OPENSSL_NO_PADDING = 3;
const int OPENSSL_PKCS1_OAEP_PADDING = 4;

const int OPENSSL_KEYTYPE_RSA = 0;
const int OPENSSL_KEYTYPE_DSA = 0;
const int OPENSSL_KEYTYPE_DH = 0;
const int OPENSSL_KEYTYPE_EC = 0;

const int OPENSSL_ALGO_DSS1 = 0;
const int OPENSSL_ALGO_SHA1 = 0;
const int OPENSSL_ALGO_SHA224 = 0;
const int OPENSSL_ALGO_SHA256 = 0;
const int OPENSSL_ALGO_SHA384 = 0;
const int OPENSSL_ALGO_SHA512 = 0;
const int OPENSSL_ALGO_RMD160 = 0;
const int OPENSSL_ALGO_MD5 = 0;
const int OPENSSL_ALGO_MD4 = 0;
const int OPENSSL_ALGO_MD2 = 0;

const int PKCS7_TEXT = 1;
const int PKCS7_BINARY = 128;
const int PKCS7_NOINTERN = 16;
const int PKCS7_NOVERIFY = 32;
const int PKCS7_NOCHAIN = 8;
const int PKCS7_NOCERTS = 2;
const int PKCS7_NOATTR = 256;
const int PKCS7_DETACHED = 64;
const int PKCS7_NOSIGS = 4;

const int OPENSSL_CIPHER_RC2_40 = 0;
const int OPENSSL_CIPHER_RC2_128 = 0;
const int OPENSSL_CIPHER_RC2_64 = 0;
const int OPENSSL_CIPHER_DES = 0;
const int OPENSSL_CIPHER_3DES = 0;

<<__PHPStdLib>>
function openssl_csr_export_to_file($csr, $outfilename, $notext = true);
<<__PHPStdLib>>
function openssl_csr_export($csr, &$out, $notext = true);
<<__PHPStdLib>>
function openssl_csr_get_public_key($csr);
<<__PHPStdLib>>
function openssl_csr_get_subject($csr, $use_shortnames = true);
<<__PHPStdLib>>
function openssl_csr_new($dn, &$privkey, $configargs = null, $extraattribs = null);
<<__PHPStdLib>>
function openssl_csr_sign($csr, $cacert, $priv_key, $days, $configargs = null, $serial = 0);
<<__PHPStdLib>>
function openssl_error_string();
<<__PHPStdLib>>
function openssl_open($sealed_data, &$open_data, $env_key, $priv_key_id);
<<__PHPStdLib>>
function openssl_pkcs12_export_to_file($x509, $filename, $priv_key, $pass, $args = null);
<<__PHPStdLib>>
function openssl_pkcs12_export($x509, &$out, $priv_key, $pass, $args = null);
<<__PHPStdLib>>
function openssl_pkcs12_read($pkcs12, &$certs, $pass);
<<__PHPStdLib>>
function openssl_pkcs7_decrypt($infilename, $outfilename, $recipcert, $recipkey = null);
<<__PHPStdLib>>
function openssl_pkcs7_encrypt($infilename, $outfilename, $recipcerts, $headers, $flags = 0, $cipherid = OPENSSL_CIPHER_RC2_40);
<<__PHPStdLib>>
function openssl_pkcs7_sign($infilename, $outfilename, $signcert, $privkey, $headers, $flags = PKCS7_DETACHED, $extracerts = null);
<<__PHPStdLib>>
function openssl_pkcs7_verify($filename, $flags, $outfilename = null, $cainfo = null, $extracerts = null, $content = null);
<<__PHPStdLib>>
function openssl_pkey_export_to_file($key, $outfilename, $passphrase = null, $configargs = null);
<<__PHPStdLib>>
function openssl_pkey_export($key, &$out, $passphrase = null, $configargs = null);
<<__PHPStdLib>>
function openssl_pkey_free($key);
<<__PHPStdLib>>
function openssl_free_key($key);
<<__PHPStdLib>>
function openssl_pkey_get_details($key);
<<__PHPStdLib>>
function openssl_pkey_get_private($key, $passphrase = null);
<<__PHPStdLib>>
function openssl_get_privatekey($key, $passphrase = null);
<<__PHPStdLib>>
function openssl_pkey_get_public($certificate);
<<__PHPStdLib>>
function openssl_get_publickey($certificate);
<<__PHPStdLib>>
function openssl_pkey_new($configargs = null);
<<__PHPStdLib>>
function openssl_private_decrypt($data, &$decrypted, $key, $padding = OPENSSL_PKCS1_PADDING);
<<__PHPStdLib>>
function openssl_private_encrypt($data, &$crypted, $key, $padding = OPENSSL_PKCS1_PADDING);
<<__PHPStdLib>>
function openssl_public_decrypt($data, &$decrypted, $key, $padding = OPENSSL_PKCS1_PADDING);
<<__PHPStdLib>>
function openssl_public_encrypt($data, &$crypted, $key, $padding = OPENSSL_PKCS1_PADDING);
<<__PHPStdLib>>
function openssl_seal($data, &$sealed_data, &$env_keys, $pub_key_ids);
<<__PHPStdLib>>
function openssl_sign($data, &$signature, $priv_key_id, $signature_alg = OPENSSL_ALGO_SHA1);
<<__PHPStdLib>>
function openssl_verify($data, $signature, $pub_key_id, $signature_alg = OPENSSL_ALGO_SHA1);
<<__PHPStdLib>>
function openssl_x509_check_private_key($cert, $key);
<<__PHPStdLib>>
function openssl_x509_checkpurpose($x509cert, $purpose, $cainfo = null, $untrustedfile = null);
<<__PHPStdLib>>
function openssl_x509_export_to_file($x509, $outfilename, $notext = true);
<<__PHPStdLib>>
function openssl_x509_export($x509, &$output, $notext = true);
<<__PHPStdLib>>
function openssl_x509_free($x509cert);
<<__PHPStdLib>>
function openssl_x509_parse($x509cert, $shortnames = true);
<<__PHPStdLib>>
function openssl_x509_read($x509certdata);
<<__PHPStdLib>>
function openssl_random_pseudo_bytes($length, &$crypto_strong = false);
<<__PHPStdLib>>
function openssl_cipher_iv_length($method);
<<__PHPStdLib>>
function openssl_encrypt($data, $method, $password, $options = 0, $iv = null, &$tag_out = null, $aad = null, $tag_length = 16);
<<__PHPStdLib>>
function openssl_decrypt($data, $method, $password, $options = 0, $iv = null, $tag = null, $aad = null);
<<__PHPStdLib>>
function openssl_digest($data, $method, $raw_output = false);
<<__PHPStdLib>>
function openssl_get_cipher_methods($aliases = false);
<<__PHPStdLib>>
function openssl_get_curve_names();
<<__PHPStdLib>>
function openssl_get_md_methods($aliases = false);
