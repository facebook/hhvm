<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int PHP_URL_SCHEME;
const int PHP_URL_HOST;
const int PHP_URL_PORT;
const int PHP_URL_USER;
const int PHP_URL_PASS;
const int PHP_URL_PATH;
const int PHP_URL_QUERY;
const int PHP_URL_FRAGMENT;

const int PHP_QUERY_RFC1738;
const int PHP_QUERY_RFC3986;

<<__PHPStdLib>>
function base64_decode(
  string $data,
  bool $strict = false,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function base64_encode(string $data)[]: string;
<<__PHPStdLib>>
function get_headers(
  string $url,
  int $format = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_meta_tags(
  string $filename,
  bool $use_include_path = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function http_build_query(
  $formdata,
  $numeric_prefix = null,
  string $arg_separator = "",
  int $enc_type = PHP_QUERY_RFC1738,
)[]: string;
<<__PHPStdLib>>
function parse_url(
  string $url,
  int $component = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function rawurldecode(string $str)[]: string;
<<__PHPStdLib>>
function rawurlencode(string $str)[]: string;
<<__PHPStdLib>>
function urldecode(string $str)[]: string;
<<__PHPStdLib>>
function urlencode(string $str)[]: string;
