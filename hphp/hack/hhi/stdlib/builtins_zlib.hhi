<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int ZLIB_ENCODING_RAW = 0;
const int ZLIB_ENCODING_GZIP = 0;
const int ZLIB_ENCODING_DEFLATE = 0;
const int ZLIB_ENCODING_ANY = 0;

const int FORCE_GZIP = 0;
const int FORCE_DEFLATE = 0;

<<__PHPStdLib>>
function gzclose(resource $zp);
<<__PHPStdLib, __Rx>>
function gzcompress(string $data, int $level = -1): mixed;
<<__PHPStdLib, __Rx>>
function gzdecode(string $data, int $length = PHP_INT_MAX): mixed;
<<__PHPStdLib, __Rx>>
function gzdeflate(string $data, int $level = -1): mixed;
<<__PHPStdLib, __Rx>>
function gzencode(string $data, int $level = -1): mixed;
<<__PHPStdLib>>
function gzeof(resource $zp);
<<__PHPStdLib>>
function gzfile(string $filename, int $use_include_path = 0);
<<__PHPStdLib>>
function gzgetc(resource $zp);
<<__PHPStdLib>>
function gzgets(resource $zp, int $length = 1024);
<<__PHPStdLib>>
function gzgetss(resource $zp, int $length = 0, string $allowable_tags = "");
<<__PHPStdLib, __Rx>>
function gzinflate(string $data, int $length = 0): mixed;
<<__PHPStdLib>>
function gzopen(string $filename, string $mode, int $use_include_path = 0);
<<__PHPStdLib>>
function gzpassthru(resource $zp);
<<__PHPStdLib>>
function gzputs(resource $zp, string $str, int $length = 0);
<<__PHPStdLib>>
function gzread(resource $zp, int $length = 0);
<<__PHPStdLib>>
function gzrewind(resource $zp);
<<__PHPStdLib>>
function gzseek(resource $zp, int $offset, int $whence = SEEK_SET);
<<__PHPStdLib>>
function gztell(resource $zp);
<<__PHPStdLib>>
function gzuncompress(string $data, int $length = 0): mixed;
<<__PHPStdLib>>
function gzwrite(resource $zp, string $str, int $length = 0);
<<__PHPStdLib, __Rx>>
function nzcompress(string $uncompressed);
<<__PHPStdLib, __Rx>>
function nzuncompress(string $compressed);
<<__PHPStdLib>>
function qlzcompress(string $data, int $level = 1);
<<__PHPStdLib>>
function qlzuncompress(string $data, int $level = 1): mixed;
<<__PHPStdLib>>
function readgzfile(string $filename, int $use_include_path = 0);
<<__PHPStdLib, __Rx>>
function zlib_decode(string $data, int $max_len = 0): mixed; // string or false
<<__PHPStdLib, __Rx>>
function zlib_encode(string $data, int $encoding, int $level = -1): mixed;
<<__PHPStdLib>>
function zlib_get_coding_type();
