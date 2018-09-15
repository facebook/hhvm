<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int JSON_ERROR_NONE = 0;
const int JSON_ERROR_DEPTH = 1;
const int JSON_ERROR_STATE_MISMATCH = 2;
const int JSON_ERROR_CTRL_CHAR = 3;
const int JSON_ERROR_SYNTAX = 4;
const int JSON_ERROR_UTF8 = 5;
const int JSON_ERROR_RECURSION = 6;
const int JSON_ERROR_INF_OR_NAN = 7;
const int JSON_ERROR_UNSUPPORTED_TYPE = 8;

// json_encode
const int JSON_HEX_TAG                 = 1<<0;
const int JSON_HEX_AMP                 = 1<<1;
const int JSON_HEX_APOS                = 1<<2;
const int JSON_HEX_QUOT                = 1<<3;
const int JSON_FORCE_OBJECT            = 1<<4;
const int JSON_NUMERIC_CHECK           = 1<<5;
const int JSON_UNESCAPED_SLASHES       = 1<<6;
const int JSON_PRETTY_PRINT            = 1<<7;
const int JSON_UNESCAPED_UNICODE       = 1<<8;
const int JSON_PARTIAL_OUTPUT_ON_ERROR = 1<<9;
const int JSON_PRESERVE_ZERO_FRACTION  = 1<<10;

// json_decode
const int JSON_OBJECT_AS_ARRAY = 1<<0;
const int JSON_BIGINT_AS_STRING = 1<<1;

const int JSON_FB_DARRAYS = 0;
const int JSON_FB_LOOSE = 0;
const int JSON_FB_UNLIMITED = 0;
const int JSON_FB_EXTRA_ESCAPES = 0;
const int JSON_FB_COLLECTIONS = 0;
const int JSON_FB_HACK_ARRAYS = 0;
const int JSON_FB_FORCE_PHP_ARRAYS = 0;
const int JSON_FB_WARN_DICTS = 0;
const int JSON_FB_WARN_PHP_ARRAYS = 0;
const int JSON_FB_WARN_EMPTY_DARRAYS = 0;
const int JSON_FB_WARN_VEC_LIKE_DARRAYS = 0;
const int JSON_FB_WARN_DICT_LIKE_DARRAYS = 0;

<<__PHPStdLib>>
function json_encode($value, $options = 0, $depth = 512);
<<__PHPStdLib>>
function json_decode($json, $assoc = false, $depth = 512, $options = 0);
<<__PHPStdLib>>
function json_last_error();
<<__PHPStdLib>>
function json_last_error_msg();
