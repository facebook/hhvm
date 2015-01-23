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
function hash($algo, $data, $raw_output = false) { }
function hash_algos() { }
function hash_init($algo, $options = 0, $key = null) { }
function hash_file($algo, $filename, $raw_output = false) { }
function hash_final($context, $raw_output = false) { }
function hash_hmac_file($algo, $filename, $key, $raw_output = false) { }
function hash_hmac($algo, $data, $key, $raw_output = false) { }
function hash_update_file($init_context, $filename, $stream_context = null) { }
function hash_update_stream($context, $handle, $length = -1) { }
function hash_update($context, $data) { }
function furchash_hphp_ext($key, $len, $nPart) { }
function furchash_hphp_ext_supported() { }
function hphp_murmurhash($key, $len, $seed) { }
