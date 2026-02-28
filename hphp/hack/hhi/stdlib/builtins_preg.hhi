<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function preg_grep(
  string $pattern,
  varray_or_darray<arraykey, mixed> $input,
  int $flags = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_grep_with_error(
  string $pattern,
  varray_or_darray<arraykey, mixed> $input,
  inout ?int $error,
  int $flags = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
/*
 * `preg_match` can actually return false if the regex fails to compile.
 * However, most code has no need to consider this possibility because their
 * regexes are known at compile time. If you are using a regex only known at
 * runtime, please handle the possibility of a false return value, and override
 * any spurious type errors with a HH_FIXME comment.
 *
 * This will eventually be fixed with more type inference magic.
 */
<<__PHPStdLib>>
function preg_match(
  string $pattern,
  string $subject,
  int $flags = 0,
  int $offset = 0,
)[]: int;
<<__PHPStdLib>>
function preg_match_with_error(
  string $pattern,
  string $subject,
  inout ?int $error,
  int $flags = 0,
  int $offset = 0,
)[]: int;
<<__PHPStdLib>>
function preg_match_with_matches(
  string $pattern,
  string $subject,
  inout dynamic $matches,
  int $flags = 0,
  int $offset = 0,
)[]: int;
<<__PHPStdLib>>
function preg_match_with_matches_and_error(
  string $pattern,
  string $subject,
  inout HH\FIXME\MISSING_PARAM_TYPE $matches,
  inout ?int $error,
  int $flags = 0,
  int $offset = 0,
)[]: int;
<<__PHPStdLib>>
function preg_match_all(
  string $pattern,
  string $subject,
  int $flags = 0,
  int $offset = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_match_all_with_error(
  string $pattern,
  string $subject,
  inout ?int $error,
  int $flags = 0,
  int $offset = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_match_all_with_matches(
  string $pattern,
  string $subject,
  inout HH\FIXME\MISSING_PARAM_TYPE $matches,
  int $flags = 0,
  int $offset = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_match_all_with_matches_and_error(
  string $pattern,
  string $subject,
  inout HH\FIXME\MISSING_PARAM_TYPE $matches,
  inout ?int $error,
  int $flags = 0,
  int $offset = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_replace(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  HH\FIXME\MISSING_PARAM_TYPE $replacement,
  HH\FIXME\MISSING_PARAM_TYPE $subject,
  int $limit = -1,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_replace_with_error(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  HH\FIXME\MISSING_PARAM_TYPE $replacement,
  HH\FIXME\MISSING_PARAM_TYPE $subject,
  inout ?int $error,
  int $limit = -1,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_replace_with_count(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  HH\FIXME\MISSING_PARAM_TYPE $replacement,
  HH\FIXME\MISSING_PARAM_TYPE $subject,
  int $limit,
  inout ?int $count,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_replace_with_count_and_error(
  HH\FIXME\MISSING_PARAM_TYPE $pattern,
  HH\FIXME\MISSING_PARAM_TYPE $replacement,
  HH\FIXME\MISSING_PARAM_TYPE $subject,
  int $limit,
  inout ?int $count,
  inout ?int $error,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_replace_callback(
  mixed $pattern,
  (function(darray<arraykey, string>)[_]: string) $callback,
  mixed $subject,
  int $limit,
  inout ?int $count,
)[ctx $callback]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_replace_callback_with_error(
  mixed $pattern,
  (function(darray<arraykey, string>)[_]: string) $callback,
  mixed $subject,
  int $limit,
  inout ?int $count,
  inout ?int $error,
)[ctx $callback]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_replace_callback_array(
  HH\FIXME\MISSING_PARAM_TYPE $patterns_and_callbacks,
  HH\FIXME\MISSING_PARAM_TYPE $subject,
  int $limit,
  inout ?int $count,
)[defaults]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_replace_callback_array_with_error(
  HH\FIXME\MISSING_PARAM_TYPE $patterns_and_callbacks,
  HH\FIXME\MISSING_PARAM_TYPE $subject,
  int $limit,
  inout ?int $count,
  inout ?int $error,
)[defaults]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_split(
  string $pattern,
  string $subject,
  HH\FIXME\MISSING_PARAM_TYPE $limit = -1,
  int $flags = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_split_with_error(
  string $pattern,
  string $subject,
  inout ?int $error,
  HH\FIXME\MISSING_PARAM_TYPE $limit = -1,
  int $flags = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function preg_get_error_message_if_invalid(
  string $pattern,
)[]: ?string;
<<__PHPStdLib>>
function preg_quote(
  string $str,
  HH\FIXME\MISSING_PARAM_TYPE $delimiter = null,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ereg_replace(
  string $pattern,
  string $replacement,
  string $str,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function eregi_replace(
  string $pattern,
  string $replacement,
  string $str,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__Deprecated('Use explode() or preg_split().'), __PHPStdLib>>
function split(
  string $pattern,
  string $str,
  int $limit = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function spliti(
  string $pattern,
  string $str,
  int $limit = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function sql_regcase(string $str): HH\FIXME\MISSING_RETURN_TYPE;

const int PREG_PATTERN_ORDER;
const int PREG_SET_ORDER;
const int PREG_OFFSET_CAPTURE;
const int PREG_SPLIT_NO_EMPTY;
const int PREG_SPLIT_DELIM_CAPTURE;
const int PREG_SPLIT_OFFSET_CAPTURE;

// Error code constants
const int PREG_NO_ERROR;
const int PREG_INTERNAL_ERROR;
const int PREG_BACKTRACK_LIMIT_ERROR;
const int PREG_RECURSION_LIMIT_ERROR;
const int PREG_BAD_UTF8_ERROR;
const int PREG_BAD_UTF8_OFFSET_ERROR;
const int PREG_BAD_REGEX_ERROR;
const int PREG_JIT_STACKLIMIT_ERROR;

const int PREG_GREP_INVERT;
const string PCRE_VERSION;
