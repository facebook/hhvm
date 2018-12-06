<?hh    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int PHP_URL_SCHEME = 0;
const int PHP_URL_HOST = 1;
const int PHP_URL_PORT = 2;
const int PHP_URL_USER = 3;
const int PHP_URL_PASS = 4;
const int PHP_URL_PATH = 5;
const int PHP_URL_QUERY = 6;
const int PHP_URL_FRAGMENT = 7;

const int PHP_QUERY_RFC1738 = 1;
const int PHP_QUERY_RFC3986 = 2;

<<__PHPStdLib, __Rx>>
function base64_decode($data, $strict = false);
<<__PHPStdLib, __Rx>>
function base64_encode($data);
<<__PHPStdLib>>
function get_headers($url, $format = 0);
<<__PHPStdLib>>
function get_meta_tags($filename, $use_include_path = false);
<<__PHPStdLib, __Rx>>
function http_build_query($formdata, $numeric_prefix = null, $arg_separator = null, $enc_type = PHP_QUERY_RFC1738): string;
<<__PHPStdLib>>
function parse_url($url, $component = -1);
<<__PHPStdLib, __Rx>>
function rawurldecode($str);
<<__PHPStdLib, __Rx>>
function rawurlencode($str);
<<__PHPStdLib, __Rx>>
function urldecode($str);
<<__PHPStdLib, __Rx>>
function urlencode($str);
