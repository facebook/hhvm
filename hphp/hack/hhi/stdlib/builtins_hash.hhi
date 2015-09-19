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
function hash(string $algo, string $data, bool $raw_output = false) { }
function hash_copy(resource $context) { }
function hash_equals(string $known_string, string $user_string): bool { }
function hash_algos(): array { }
function hash_init(string $algo, int $options = 0, string $key = null) { }
function hash_file(string $algo, string $filename, bool $raw_output = false) { }
function hash_final(resource $context, bool $raw_output = false) { }
function hash_hmac_file(string $algo, string $filename, string $key, bool $raw_output = false) { }
function hash_hmac(string $algo, string $data, string $key, bool $raw_output = false) { }
function hash_update_file(resource $init_context, string $filename, resource $stream_context = null) { }
function hash_update_stream(resource $context, resource $handle, int $length = -1) { }
function hash_update(resource $context, string $data) { }
function furchash_hphp_ext($key, $len, $nPart) { }
function furchash_hphp_ext_supported() { }
function hphp_murmurhash($key, $len, $seed) { }
function hash_pbkdf2(string $algo, string $password, string $salt, int $iterations, int $length = 0, bool $raw_output = false) { }
