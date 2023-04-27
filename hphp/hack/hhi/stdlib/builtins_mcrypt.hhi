<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int MCRYPT_ENCRYPT;
const int MCRYPT_DECRYPT;
const int MCRYPT_DEV_RANDOM;
const int MCRYPT_DEV_URANDOM;
const int MCRYPT_RAND;
const string MCRYPT_3DES;
const string MCRYPT_ARCFOUR_IV;
const string MCRYPT_ARCFOUR;
const string MCRYPT_BLOWFISH;
const string MCRYPT_BLOWFISH_COMPAT;
const string MCRYPT_CAST_128;
const string MCRYPT_CAST_256;
const string MCRYPT_CRYPT;
const string MCRYPT_DES;
const string MCRYPT_ENIGNA;
const string MCRYPT_GOST;
const string MCRYPT_LOKI97;
const string MCRYPT_PANAMA;
const string MCRYPT_RC2;
const string MCRYPT_RIJNDAEL_128;
const string MCRYPT_RIJNDAEL_192;
const string MCRYPT_RIJNDAEL_256;
const string MCRYPT_SAFER64;
const string MCRYPT_SAFER128;
const string MCRYPT_SAFERPLUS;
const string MCRYPT_SERPENT;
const string MCRYPT_THREEWAY;
const string MCRYPT_TRIPLEDES;
const string MCRYPT_TWOFISH;
const string MCRYPT_WAKE;
const string MCRYPT_XTEA;
const string MCRYPT_IDEA;
const string MCRYPT_MARS;
const string MCRYPT_RC6;
const string MCRYPT_SKIPJACK;
const string MCRYPT_MODE_CBC;
const string MCRYPT_MODE_CFB;
const string MCRYPT_MODE_ECB;
const string MCRYPT_MODE_NOFB;
const string MCRYPT_MODE_OFB;
const string MCRYPT_MODE_STREAM;

<<__PHPStdLib>>
function mcrypt_module_open(
  string $algorithm,
  string $algorithm_directory,
  string $mode,
  string $mode_directory,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_module_close(resource $td): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_list_algorithms(
  string $lib_dir = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_list_modes(string $lib_dir = ""): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_module_get_algo_block_size(
  string $algorithm,
  string $lib_dir = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_module_get_algo_key_size(
  string $algorithm,
  string $lib_dir = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_module_get_supported_key_sizes(
  string $algorithm,
  string $lib_dir = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_module_is_block_algorithm_mode(
  string $mode,
  string $lib_dir = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_module_is_block_algorithm(
  string $algorithm,
  string $lib_dir = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_module_is_block_mode(
  string $mode,
  string $lib_dir = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_module_self_test(
  string $algorithm,
  string $lib_dir = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_create_iv(
  int $size,
  int $source = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_encrypt(
  string $cipher,
  string $key,
  string $data,
  string $mode,
  HH\FIXME\MISSING_PARAM_TYPE $iv = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_decrypt(
  string $cipher,
  string $key,
  string $data,
  string $mode,
  HH\FIXME\MISSING_PARAM_TYPE $iv = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_cbc(
  string $cipher,
  string $key,
  string $data,
  HH\FIXME\MISSING_PARAM_TYPE $mode,
  HH\FIXME\MISSING_PARAM_TYPE $iv = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_cfb(
  string $cipher,
  string $key,
  string $data,
  HH\FIXME\MISSING_PARAM_TYPE $mode,
  HH\FIXME\MISSING_PARAM_TYPE $iv = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_ecb(
  string $cipher,
  string $key,
  string $data,
  HH\FIXME\MISSING_PARAM_TYPE $mode,
  HH\FIXME\MISSING_PARAM_TYPE $iv = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_ofb(
  string $cipher,
  string $key,
  string $data,
  HH\FIXME\MISSING_PARAM_TYPE $mode,
  HH\FIXME\MISSING_PARAM_TYPE $iv = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_get_block_size(
  string $cipher,
  string $mode,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_get_cipher_name(string $cipher): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_get_iv_size(
  string $cipher,
  string $mode,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_get_key_size(
  string $cipher,
  string $module,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_enc_get_algorithms_name(
  resource $td,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_enc_get_block_size(resource $td): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_enc_get_iv_size(resource $td): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_enc_get_key_size(resource $td): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_enc_get_modes_name(resource $td): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_enc_get_supported_key_sizes(
  resource $td,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_enc_is_block_algorithm_mode(
  resource $td,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_enc_is_block_algorithm(
  resource $td,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_enc_is_block_mode(resource $td): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_enc_self_test(resource $td): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_generic(
  resource $td,
  string $data,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_generic_init(
  resource $td,
  string $key,
  string $iv,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mdecrypt_generic(
  resource $td,
  string $data,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_generic_deinit(resource $td): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mcrypt_generic_end(resource $td): HH\FIXME\MISSING_RETURN_TYPE;
