<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib, __Rx>>
function hash(string $algo, string $data, bool $raw_output = false);
<<__PHPStdLib, __Rx>>
function hash_algos();
<<__PHPStdLib>>
function hash_init(string $algo, int $options = 0, string $key = "");
<<__PHPStdLib>>
function hash_copy(resource $algo) { }
<<__PHPStdLib>>
function hash_equals(string $known_string, string $user_string);
<<__PHPStdLib>>
function hash_file(string $algo, string $filename, bool $raw_output = false);
<<__PHPStdLib>>
function hash_final(resource $context, bool $raw_output = false);
<<__PHPStdLib>>
function hash_hmac_file($algo, $filename, $key, $raw_output = false);
<<__PHPStdLib, __Rx>>
function hash_hmac($algo, $data, $key, $raw_output = false);
<<__PHPStdLib>>
function hash_pbkdf2(string $algo, string $password, string $salt, int $iterations, int $length = 0, bool $raw_output = false);
<<__PHPStdLib>>
function hash_update_file($init_context, string $filename, $stream_context = null);
<<__PHPStdLib>>
function hash_update_stream($context, $handle, int $length = -1);
<<__PHPStdLib>>
function hash_update(resource $context, string $data);
<<__PHPStdLib, __Rx>>
function furchash_hphp_ext(string $key, int $len, int $nPart);
<<__PHPStdLib, __Rx>>
function furchash_hphp_ext_supported();
<<__PHPStdLib, __Rx>>
function hphp_murmurhash(string $key, int $len, int $seed);
