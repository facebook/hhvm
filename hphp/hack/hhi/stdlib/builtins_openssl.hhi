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
function openssl_csr_export_to_file($csr, string $outfilename, bool $notext = true);
<<__PHPStdLib>>
function openssl_csr_export($csr, inout $out, bool $notext = true);
<<__PHPStdLib>>
function openssl_csr_get_public_key($csr);
<<__PHPStdLib>>
function openssl_csr_get_subject($csr, bool $use_shortnames = true);
<<__PHPStdLib>>
function openssl_csr_new($dn, inout $privkey, $configargs = null, $extraattribs = null);
<<__PHPStdLib>>
function openssl_csr_sign($csr, $cacert, $priv_key, int $days, $configargs = null, int $serial = 0);
<<__PHPStdLib>>
function openssl_error_string();
<<__PHPStdLib>>
function openssl_open(string $sealed_data, inout $open_data, string $env_key, $priv_key_id);
<<__PHPStdLib>>
function openssl_pkcs12_export_to_file($x509, string $filename, $priv_key, string $pass, $args = null);
<<__PHPStdLib>>
function openssl_pkcs12_export($x509, inout $out, $priv_key, string $pass, $args = null);
<<__PHPStdLib>>
function openssl_pkcs12_read(string $pkcs12, inout $certs, string $pass);
<<__PHPStdLib>>
function openssl_pkcs7_decrypt(string $infilename, string $outfilename, $recipcert, $recipkey = null);
<<__PHPStdLib>>
function openssl_pkcs7_encrypt(string $infilename, string $outfilename, $recipcerts, $headers, int $flags = 0, int $cipherid = OPENSSL_CIPHER_RC2_40);
<<__PHPStdLib>>
function openssl_pkcs7_sign(string $infilename, string $outfilename, $signcert, $privkey, $headers, int $flags = PKCS7_DETACHED, string $extracerts = "");
<<__PHPStdLib>>
function openssl_pkcs7_verify(string $filename, int $flags, $outfilename = null, $cainfo = null, $extracerts = null, $content = null);
<<__PHPStdLib>>
function openssl_pkey_export_to_file($key, string $outfilename, string $passphrase = "", $configargs = null);
<<__PHPStdLib>>
function openssl_pkey_export($key, inout $out, string $passphrase = "", $configargs = null);
<<__PHPStdLib>>
function openssl_pkey_free(resource $key);
<<__PHPStdLib>>
function openssl_free_key(resource $key);
<<__PHPStdLib>>
function openssl_pkey_get_details(resource $key);
<<__PHPStdLib>>
function openssl_pkey_get_private($key, string $passphrase = "");
<<__PHPStdLib>>
function openssl_get_privatekey($key, $passphrase = null);
<<__PHPStdLib>>
function openssl_pkey_get_public($certificate);
<<__PHPStdLib>>
function openssl_get_publickey($certificate);
<<__PHPStdLib>>
function openssl_pkey_new($configargs = null);
<<__PHPStdLib>>
function openssl_private_decrypt(string $data, inout $decrypted, $key, int $padding = OPENSSL_PKCS1_PADDING);
<<__PHPStdLib>>
function openssl_private_encrypt(string $data, inout $crypted, $key, int $padding = OPENSSL_PKCS1_PADDING);
<<__PHPStdLib>>
function openssl_public_decrypt(string $data, inout $decrypted, $key, int $padding = OPENSSL_PKCS1_PADDING);
<<__PHPStdLib>>
function openssl_public_encrypt(string $data, inout $crypted, $key, int $padding = OPENSSL_PKCS1_PADDING);
<<__PHPStdLib>>
function openssl_seal(string $data, inout $sealed_data, inout $env_keys,
                      array $pub_key_ids, string $method, inout $iv);
<<__PHPStdLib>>
function openssl_sign(string $data, inout $signature, $priv_key_id, $signature_alg = OPENSSL_ALGO_SHA1);
<<__PHPStdLib>>
function openssl_verify(string $data, string $signature, $pub_key_id, $signature_alg = OPENSSL_ALGO_SHA1);
<<__PHPStdLib>>
function openssl_x509_check_private_key($cert, $key);
<<__PHPStdLib>>
function openssl_x509_checkpurpose($x509cert, int $purpose, varray $cainfo = varray[], string $untrustedfile = "");
<<__PHPStdLib>>
function openssl_x509_export_to_file($x509, string $outfilename, bool $notext = true);
<<__PHPStdLib>>
function openssl_x509_export($x509, inout $output, bool $notext = true);
<<__PHPStdLib>>
function openssl_x509_free(resource $x509cert);
<<__PHPStdLib>>
function openssl_x509_parse($x509cert, bool $shortnames = true);
<<__PHPStdLib>>
function openssl_x509_read($x509certdata);
<<__PHPStdLib>>
function openssl_random_pseudo_bytes(int $length, inout $crypto_strong);
<<__PHPStdLib>>
function openssl_cipher_iv_length(string $method);
<<__PHPStdLib>>
function openssl_encrypt(string $data, string $method, string $password, int $options = 0, string $iv = "", string $aad = "", int $tag_length = 16);
<<__PHPStdLib>>
function openssl_encrypt_with_tag(string $data, string $method, string $password, int $options, string $iv, inout $tag_out, string $aad = "", int $tag_length = 16);
<<__PHPStdLib>>
function openssl_decrypt(string $data, string $method, string $password, int $options = 0, string $iv = "", string $tag = "", string $aad = "");
<<__PHPStdLib>>
function openssl_digest(string $data, string $method, bool $raw_output = false);
<<__PHPStdLib>>
function openssl_get_cipher_methods(bool $aliases = false);
<<__PHPStdLib>>
function openssl_get_curve_names();
<<__PHPStdLib>>
function openssl_get_md_methods(bool $aliases = false);
