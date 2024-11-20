<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// Error code constants
// No error has occurred
const int JSON_ERROR_NONE;
// The maximum stack depth has been exceeded
const int JSON_ERROR_DEPTH;
// Invalid or malformed JSON
const int JSON_ERROR_STATE_MISMATCH;
// Control character error, possibly incorrectly encoded
const int JSON_ERROR_CTRL_CHAR;
// Syntax error
const int JSON_ERROR_SYNTAX;
// Malformed UTF-8 characters, possibly incorrectly encoded
const int JSON_ERROR_UTF8;
// One or more recursive references in the value to be encoded
const int JSON_ERROR_RECURSION;
// One or more NAN or INF values in the value to be encoded
const int JSON_ERROR_INF_OR_NAN;
// A value of a type that cannot be encoded was given
const int JSON_ERROR_UNSUPPORTED_TYPE;

// json_encode
const int JSON_HEX_TAG;
const int JSON_HEX_AMP;
const int JSON_HEX_APOS;
const int JSON_HEX_QUOT;
const int JSON_FORCE_OBJECT;
const int JSON_NUMERIC_CHECK;
const int JSON_UNESCAPED_SLASHES;
const int JSON_PRETTY_PRINT;
const int JSON_UNESCAPED_UNICODE;
const int JSON_PARTIAL_OUTPUT_ON_ERROR;
const int JSON_PRESERVE_ZERO_FRACTION;

// json_decode
const int JSON_OBJECT_AS_ARRAY;
const int JSON_BIGINT_AS_STRING;

const int JSON_FB_DARRAYS;
const int JSON_FB_LOOSE;
const int JSON_FB_UNLIMITED;
const int JSON_FB_EXTRA_ESCAPES;
const int JSON_FB_COLLECTIONS;
const int JSON_FB_HACK_ARRAYS;
const int JSON_FB_FORCE_PHP_ARRAYS;
const int JSON_FB_WARN_DICTS;
const int JSON_FB_WARN_PHP_ARRAYS;
const int JSON_FB_WARN_EMPTY_DARRAYS;
const int JSON_FB_WARN_VEC_LIKE_DARRAYS;
const int JSON_FB_WARN_DICT_LIKE_DARRAYS;
const int JSON_FB_IGNORE_LATEINIT;
const int JSON_FB_THRIFT_SIMPLE_JSON;
const int JSON_FB_WARN_KEYSETS;
const int JSON_FB_FORCE_HACK_ARRAYS;
const int JSON_FB_SORT_KEYS;
const int JSON_FB_FULL_FLOAT_PRECISION;

<<__PHPStdLib>>
function json_encode(
  mixed $value,
  int $options = 0,
  int $depth = 512,
)[defaults]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function json_encode_with_error(
  mixed $value,
  inout ?(int, string) $error,
  int $options = 0,
  int $depth = 512,
)[defaults]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function json_encode_pure(
  mixed $value,
  inout ?(int, string) $error,
  int $options = 0,
  int $depth = 512,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function json_decode(
  string $json,
  bool $assoc = false,
  int $depth = 512,
  int $options = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function json_decode_with_error(
  string $json,
  inout ?(int, string) $error,
  bool $assoc = false,
  int $depth = 512,
  int $options = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
