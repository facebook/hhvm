<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int MCRYPT_ENCRYPT = 0;
const int MCRYPT_DECRYPT = 1;
const int MCRYPT_DEV_RANDOM = 0;
const int MCRYPT_DEV_URANDOM = 1;
const int MCRYPT_RAND = 2;
const string MCRYPT_3DES = 'tripledes';
const string MCRYPT_ARCFOUR_IV = 'arcfour-iv';
const string MCRYPT_ARCFOUR = 'arcfour';
const string MCRYPT_BLOWFISH = 'blowfish';
const string MCRYPT_BLOWFISH_COMPAT = 'blowfish-compat';
const string MCRYPT_CAST_128 = 'cast-128';
const string MCRYPT_CAST_256 = 'cast-256';
const string MCRYPT_CRYPT = 'crypt';
const string MCRYPT_DES = 'des';
const string MCRYPT_ENIGNA = 'crypt';
const string MCRYPT_GOST = 'gost';
const string MCRYPT_LOKI97 = 'loki97';
const string MCRYPT_PANAMA = 'panama';
const string MCRYPT_RC2 = 'rc2';
const string MCRYPT_RIJNDAEL_128 = 'rijndael-128';
const string MCRYPT_RIJNDAEL_192 = 'rijndael-192';
const string MCRYPT_RIJNDAEL_256 = 'rijndael-256';
const string MCRYPT_SAFER64 = 'safer-sk64';
const string MCRYPT_SAFER128 = 'safer-sk128';
const string MCRYPT_SAFERPLUS = 'saferplus';
const string MCRYPT_SERPENT = 'serpent';
const string MCRYPT_THREEWAY = 'threeway';
const string MCRYPT_TRIPLEDES = 'tripledes';
const string MCRYPT_TWOFISH = 'twofish';
const string MCRYPT_WAKE = 'wake';
const string MCRYPT_XTEA = 'xtea';
const string MCRYPT_IDEA = 'idea';
const string MCRYPT_MARS = 'mars';
const string MCRYPT_RC6 = 'rc6';
const string MCRYPT_SKIPJACK = 'skipjack';
const string MCRYPT_MODE_CBC = 'cbc';
const string MCRYPT_MODE_CFB = 'cfb';
const string MCRYPT_MODE_ECB = 'ecb';
const string MCRYPT_MODE_NOFB = 'nofb';
const string MCRYPT_MODE_OFB = 'ofb';
const string MCRYPT_MODE_STREAM = 'stream';

<<__PHPStdLib>>
function mcrypt_module_open(string $algorithm, string $algorithm_directory, string $mode, string $mode_directory);
<<__PHPStdLib>>
function mcrypt_module_close(resource $td);
<<__PHPStdLib>>
function mcrypt_list_algorithms(string $lib_dir = "");
<<__PHPStdLib>>
function mcrypt_list_modes(string $lib_dir = "");
<<__PHPStdLib>>
function mcrypt_module_get_algo_block_size(string $algorithm, string $lib_dir = "");
<<__PHPStdLib>>
function mcrypt_module_get_algo_key_size(string $algorithm, string $lib_dir = "");
<<__PHPStdLib>>
function mcrypt_module_get_supported_key_sizes(string $algorithm, string $lib_dir = "");
<<__PHPStdLib>>
function mcrypt_module_is_block_algorithm_mode(string $mode, string $lib_dir = "");
<<__PHPStdLib>>
function mcrypt_module_is_block_algorithm(string $algorithm, string $lib_dir = "");
<<__PHPStdLib>>
function mcrypt_module_is_block_mode(string $mode, string $lib_dir = "");
<<__PHPStdLib>>
function mcrypt_module_self_test(string $algorithm, string $lib_dir = "");
<<__PHPStdLib>>
function mcrypt_create_iv(int $size, int $source = 0);
<<__PHPStdLib>>
function mcrypt_encrypt(string $cipher, string $key, string $data, string $mode, $iv = null);
<<__PHPStdLib>>
function mcrypt_decrypt(string $cipher, string $key, string $data, string $mode, $iv = null);
<<__PHPStdLib>>
function mcrypt_cbc(string $cipher, string $key, string $data, $mode, $iv = null);
<<__PHPStdLib>>
function mcrypt_cfb(string $cipher, string $key, string $data, $mode, $iv = null);
<<__PHPStdLib>>
function mcrypt_ecb(string $cipher, string $key, string $data, $mode, $iv = null);
<<__PHPStdLib>>
function mcrypt_ofb(string $cipher, string $key, string $data, $mode, $iv = null);
<<__PHPStdLib>>
function mcrypt_get_block_size(string $cipher, string $mode);
<<__PHPStdLib>>
function mcrypt_get_cipher_name(string $cipher);
<<__PHPStdLib>>
function mcrypt_get_iv_size(string $cipher, string $mode);
<<__PHPStdLib>>
function mcrypt_get_key_size(string $cipher, string $module);
<<__PHPStdLib>>
function mcrypt_enc_get_algorithms_name(resource $td);
<<__PHPStdLib>>
function mcrypt_enc_get_block_size(resource $td);
<<__PHPStdLib>>
function mcrypt_enc_get_iv_size(resource $td);
<<__PHPStdLib>>
function mcrypt_enc_get_key_size(resource $td);
<<__PHPStdLib>>
function mcrypt_enc_get_modes_name(resource $td);
<<__PHPStdLib>>
function mcrypt_enc_get_supported_key_sizes(resource $td);
<<__PHPStdLib>>
function mcrypt_enc_is_block_algorithm_mode(resource $td);
<<__PHPStdLib>>
function mcrypt_enc_is_block_algorithm(resource $td);
<<__PHPStdLib>>
function mcrypt_enc_is_block_mode(resource $td);
<<__PHPStdLib>>
function mcrypt_enc_self_test(resource $td);
<<__PHPStdLib>>
function mcrypt_generic(resource $td, string $data);
<<__PHPStdLib>>
function mcrypt_generic_init(resource $td, string $key, string $iv);
<<__PHPStdLib>>
function mdecrypt_generic(resource $td, string $data);
<<__PHPStdLib>>
function mcrypt_generic_deinit(resource $td);
<<__PHPStdLib>>
function mcrypt_generic_end(resource $td);
