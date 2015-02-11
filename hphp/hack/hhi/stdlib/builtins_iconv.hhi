<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

const string ICONV_IMPL = '';
const string ICONV_VERSION = '';

const int ICONV_MIME_DECODE_STRICT = 0b001;
const int ICONV_MIME_DECODE_CONTINUE_ON_ERROR = 0b010;

function iconv_mime_encode($field_name, $field_value, $preferences = null);
function iconv_mime_decode($encoded_string, $mode = 0, $charset = null);
function iconv_mime_decode_headers($encoded_headers, $mode = 0, $charset = null);
function iconv_get_encoding($type = "all");
function iconv_set_encoding($type, $charset);
function iconv($in_charset, $out_charset, $str);
function iconv_strlen($str, $charset = null);
function iconv_strpos($haystack, $needle, $offset = 0, $charset = null);
function iconv_strrpos($haystack, $needle, $charset = null);
function iconv_substr($str, $offset, $length = PHP_INT_MAX, $charset = null);
function ob_iconv_handler($contents, $status);
