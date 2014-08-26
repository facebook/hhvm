<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
define('MCRYPT_ENCRYPT', 0);
define('MCRYPT_DECRYPT', 1);
define('MCRYPT_DEV_RANDOM', 0);
define('MCRYPT_DEV_URANDOM', 1);
define('MCRYPT_RAND', 2);
define('MCRYPT_3DES', 'tripledes');
define('MCRYPT_ARCFOUR_IV', 'arcfour-iv');
define('MCRYPT_ARCFOUR', 'arcfour');
define('MCRYPT_BLOWFISH', 'blowfish');
define('MCRYPT_BLOWFISH_COMPAT', 'blowfish-compat');
define('MCRYPT_CAST_128', 'cast-128');
define('MCRYPT_CAST_256', 'cast-256');
define('MCRYPT_CRYPT', 'crypt');
define('MCRYPT_DES', 'des');
define('MCRYPT_ENIGNA', 'crypt');
define('MCRYPT_GOST', 'gost');
define('MCRYPT_LOKI97', 'loki97');
define('MCRYPT_PANAMA', 'panama');
define('MCRYPT_RC2', 'rc2');
define('MCRYPT_RIJNDAEL_128', 'rijndael-128');
define('MCRYPT_RIJNDAEL_192', 'rijndael-192');
define('MCRYPT_RIJNDAEL_256', 'rijndael-256');
define('MCRYPT_SAFER64', 'safer-sk64');
define('MCRYPT_SAFER128', 'safer-sk128');
define('MCRYPT_SAFERPLUS', 'saferplus');
define('MCRYPT_SERPENT', 'serpent');
define('MCRYPT_THREEWAY', 'threeway');
define('MCRYPT_TRIPLEDES', 'tripledes');
define('MCRYPT_TWOFISH', 'twofish');
define('MCRYPT_WAKE', 'wake');
define('MCRYPT_XTEA', 'xtea');
define('MCRYPT_IDEA', 'idea');
define('MCRYPT_MARS', 'mars');
define('MCRYPT_RC6', 'rc6');
define('MCRYPT_SKIPJACK', 'skipjack');
define('MCRYPT_MODE_CBC', 'cbc');
define('MCRYPT_MODE_CFB', 'cfb');
define('MCRYPT_MODE_ECB', 'ecb');
define('MCRYPT_MODE_NOFB', 'nofb');
define('MCRYPT_MODE_OFB', 'ofb');
define('MCRYPT_MODE_STREAM', 'stream');
function mcrypt_module_open($algorithm, $algorithm_directory, $mode, $mode_directory) { }
function mcrypt_module_close($td) { }
function mcrypt_list_algorithms($lib_dir = null) { }
function mcrypt_list_modes($lib_dir = null) { }
function mcrypt_module_get_algo_block_size($algorithm, $lib_dir = null) { }
function mcrypt_module_get_algo_key_size($algorithm, $lib_dir = null) { }
function mcrypt_module_get_supported_key_sizes($algorithm, $lib_dir = null) { }
function mcrypt_module_is_block_algorithm_mode($mode, $lib_dir = null) { }
function mcrypt_module_is_block_algorithm($algorithm, $lib_dir = null) { }
function mcrypt_module_is_block_mode($mode, $lib_dir = null) { }
function mcrypt_module_self_test($algorithm, $lib_dir = null) { }
function mcrypt_create_iv($size, $source = 0) { }
function mcrypt_encrypt($cipher, $key, $data, $mode, $iv = null) { }
function mcrypt_decrypt($cipher, $key, $data, $mode, $iv = null) { }
function mcrypt_cbc($cipher, $key, $data, $mode, $iv = null) { }
function mcrypt_cfb($cipher, $key, $data, $mode, $iv = null) { }
function mcrypt_ecb($cipher, $key, $data, $mode, $iv = null) { }
function mcrypt_ofb($cipher, $key, $data, $mode, $iv = null) { }
function mcrypt_get_block_size($cipher, $module = null) { }
function mcrypt_get_cipher_name($cipher) { }
function mcrypt_get_iv_size($cipher, $mode) { }
function mcrypt_get_key_size($cipher, $module) { }
function mcrypt_enc_get_algorithms_name($td) { }
function mcrypt_enc_get_block_size($td) { }
function mcrypt_enc_get_iv_size($td) { }
function mcrypt_enc_get_key_size($td) { }
function mcrypt_enc_get_modes_name($td) { }
function mcrypt_enc_get_supported_key_sizes($td) { }
function mcrypt_enc_is_block_algorithm_mode($td) { }
function mcrypt_enc_is_block_algorithm($td) { }
function mcrypt_enc_is_block_mode($td) { }
function mcrypt_enc_self_test($td) { }
function mcrypt_generic($td, $data) { }
function mcrypt_generic_init($td, $key, $iv) { }
function mdecrypt_generic($td, $data) { }
function mcrypt_generic_deinit($td) { }
function mcrypt_generic_end($td) { }
