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
function iconv_mime_encode($field_name, $field_value, $preferences = null);
<<__PHPStdLib>>
function iconv_mime_decode($encoded_string, $mode = 0, $charset = null);
<<__PHPStdLib>>
function iconv_mime_decode_headers($encoded_headers, $mode = 0, $charset = null);
<<__PHPStdLib>>
function iconv_get_encoding($type = "all");
<<__PHPStdLib>>
function iconv_set_encoding($type, $charset);
<<__PHPStdLib>>
function iconv($in_charset, $out_charset, $str);
<<__PHPStdLib>>
function iconv_strlen($str, $charset = null);
<<__PHPStdLib>>
function iconv_strpos($haystack, $needle, $offset = 0, $charset = null);
<<__PHPStdLib>>
function iconv_strrpos($haystack, $needle, $charset = null);
<<__PHPStdLib>>
function iconv_substr($str, $offset, $length = PHP_INT_MAX, $charset = null);
<<__PHPStdLib>>
function ob_iconv_handler($contents, $status);
