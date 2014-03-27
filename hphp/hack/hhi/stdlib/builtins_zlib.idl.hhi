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
function readgzfile($filename, $use_include_path = false) { }
function gzfile($filename, $use_include_path = false) { }
function zlib_get_coding_type() { }
function gzopen($filename, $mode, $use_include_path = false) { }
function gzclose($zp) { }
function gzrewind($zp) { }
function gzeof($zp) { }
function gzgetc($zp) { }
function gzgets($zp, $length = 1024) { }
function gzgetss($zp, $length = 0, $allowable_tags = null) { }
function gzread($zp, $length = 0) { }
function gzpassthru($zp) { }
function gzseek($zp, $offset, $whence = SEEK_SET) { }
function gztell($zp) { }
function gzwrite($zp, $str, $length = 0) { }
function gzputs($zp, $str, $length = 0) { }
function qlzcompress($data, $level = 1) { }
function qlzuncompress($data, $level = 1) { }
function sncompress($data) { }
function snuncompress($data) { }
function nzcompress($uncompressed) { }
function nzuncompress($compressed) { }
function lz4compress($uncompressed) { }
function lz4hccompress($uncompressed) { }
function lz4uncompress($compressed) { }
