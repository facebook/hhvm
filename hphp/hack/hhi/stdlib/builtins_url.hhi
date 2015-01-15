<?hh // decl    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

function base64_decode($data, $strict = false);
function base64_encode($data);
function get_headers($url, $format = 0);
function get_meta_tags($filename, $use_include_path = false);
function http_build_query($formdata, $numeric_prefix = null, $arg_separator = null): string;
function parse_url($url, $component = -1);
function rawurldecode($str);
function rawurlencode($str);
function urldecode($str);
function urlencode($str);
