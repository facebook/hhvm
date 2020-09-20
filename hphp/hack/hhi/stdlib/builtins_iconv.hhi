<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const string ICONV_IMPL = '';
const string ICONV_VERSION = '';

const int ICONV_MIME_DECODE_STRICT = 0b001;
const int ICONV_MIME_DECODE_CONTINUE_ON_ERROR = 0b010;

<<__PHPStdLib>>
function iconv_mime_encode(string $field_name, string $field_value, $preferences = null);
<<__PHPStdLib>>
function iconv_mime_decode(string $encoded_string, int $mode = 0, $charset = null);
<<__PHPStdLib>>
function iconv_mime_decode_headers(string $encoded_headers, int $mode = 0, $charset = null);
<<__PHPStdLib>>
function iconv_get_encoding(string $type = "all");
<<__PHPStdLib>>
function iconv_set_encoding(string $type, string $charset);
<<__PHPStdLib>>
function iconv(string $in_charset, string $out_charset, string $str);
<<__PHPStdLib>>
function iconv_strlen(string $str, $charset = null);
<<__PHPStdLib>>
function iconv_strpos(string $haystack, string $needle, int $offset = 0, $charset = null);
<<__PHPStdLib>>
function iconv_strrpos(string $haystack, string $needle, $charset = null);
<<__PHPStdLib>>
function iconv_substr(string $str, int $offset, int $length = PHP_INT_MAX, $charset = null);
<<__PHPStdLib>>
function ob_iconv_handler(string $contents, int $status);
