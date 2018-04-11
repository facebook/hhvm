<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function hash($algo, $data, $raw_output = false) { }
<<__PHPStdLib>>
function hash_algos() { }
<<__PHPStdLib>>
function hash_init($algo, $options = 0, $key = null) { }
<<__PHPStdLib>>
function hash_file($algo, $filename, $raw_output = false) { }
<<__PHPStdLib>>
function hash_final($context, $raw_output = false) { }
<<__PHPStdLib>>
function hash_hmac_file($algo, $filename, $key, $raw_output = false) { }
<<__PHPStdLib>>
function hash_hmac($algo, $data, $key, $raw_output = false) { }
<<__PHPStdLib>>
function hash_update_file($init_context, $filename, $stream_context = null) { }
<<__PHPStdLib>>
function hash_update_stream($context, $handle, $length = -1) { }
<<__PHPStdLib>>
function hash_update($context, $data) { }
<<__PHPStdLib>>
function furchash_hphp_ext($key, $len, $nPart) { }
<<__PHPStdLib>>
function furchash_hphp_ext_supported() { }
<<__PHPStdLib>>
function hphp_murmurhash($key, $len, $seed) { }
