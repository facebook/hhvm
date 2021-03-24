<?hh    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function preg_grep(string $pattern, varray_or_darray $input, int $flags = 0);
<<__PHPStdLib>>
function preg_grep_with_error(
  string $pattern,
  varray_or_darray $input,
  inout ?int $error,
  int $flags = 0,
)[];
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
function preg_match(string $pattern, string $subject,
                    int $flags = 0, int $offset = 0): int;
<<__PHPStdLib>>
function preg_match_with_error(
  string $pattern,
  string $subject,
  inout ?int $error,
  int $flags = 0,
  int $offset = 0,
)[]: int;
<<__PHPStdLib>> // not pure: puts error code in a global variable
function preg_match_with_matches(string $pattern, string $subject, inout $matches,
                                 int $flags = 0, int $offset = 0): int;
<<__PHPStdLib>>
function preg_match_with_matches_and_error(
  string $pattern,
  string $subject,
  inout $matches,
  inout ?int $error,
  int $flags = 0,
  int $offset = 0,
)[]: int;
<<__PHPStdLib>>
function preg_match_all(string $pattern, string $subject,
                        int $flags = 0, int $offset = 0);
<<__PHPStdLib>>
function preg_match_all_with_error(
  string $pattern,
  string $subject,
  inout ?int $error,
  int $flags = 0,
  int $offset = 0,
)[];
<<__PHPStdLib>>
function preg_match_all_with_matches(string $pattern, string $subject, inout $matches,
                                     int $flags = 0, int $offset = 0);
<<__PHPStdLib>>
function preg_match_all_with_matches_and_error(
  string $pattern,
  string $subject,
  inout $matches,
  inout ?int $error,
  int $flags = 0,
  int $offset = 0,
)[];
<<__PHPStdLib>>
function preg_replace($pattern, $replacement, $subject, int $limit = -1);
<<__PHPStdLib>>
function preg_replace_with_error(
  $pattern,
  $replacement,
  $subject,
  inout ?int $error,
  int $limit = -1,
)[];
<<__PHPStdLib>>
function preg_replace_with_count($pattern, $replacement, $subject, int $limit,
                                 inout ?int $count);
<<__PHPStdLib>>
function preg_replace_with_count_and_error(
  $pattern,
  $replacement,
  $subject,
  int $limit,
  inout ?int $count,
  inout ?int $error,
)[];
<<__PHPStdLib>>
function preg_replace_callback($pattern, $callback, $subject, int $limit,
                               inout ?int $count);
<<__PHPStdLib>>
function preg_replace_callback_with_error(
  $pattern,
  $callback,
  $subject,
  int $limit,
  inout ?int $count,
  inout ?int $error,
);
<<__PHPStdLib>>
function preg_replace_callback_array($patterns_and_callbacks, $subject,
                                     int $limit, inout ?int $count);
<<__PHPStdLib>>
function preg_replace_callback_array_with_error(
  $patterns_and_callbacks,
  $subject,
  int $limit,
  inout ?int $count,
  inout ?int $error,
);
<<__PHPStdLib>>
function preg_split(string $pattern, string $subject, $limit = -1, int $flags = 0)[];
<<__PHPStdLib>>
function preg_split_with_error(
  string $pattern,
  string $subject,
  inout ?int $error,
  $limit = -1,
  int $flags = 0,
)[];
<<__PHPStdLib>>
function preg_quote(string $str, $delimiter = null)[];
<<__PHPStdLib>>
function preg_last_error();
<<__PHPStdLib>>
function ereg_replace(string $pattern, string $replacement, string $str);
<<__PHPStdLib>>
function eregi_replace(string $pattern, string $replacement, string $str);
<<__Deprecated('Use explode() or preg_split().'), __PHPStdLib>>
function split(string $pattern, string $str, int $limit = -1);
<<__PHPStdLib>>
function spliti(string $pattern, string $str, int $limit = -1);
<<__PHPStdLib>>
function sql_regcase(string $str);

const int PREG_PATTERN_ORDER = 0;
const int PREG_SET_ORDER = 0;
const int PREG_OFFSET_CAPTURE = 0;
const int PREG_FB_HACK_ARRAYS = 0;
const int PREG_HACK_ARR = 0; // legacy, equivalent to FB_HACK_ARRAYS
const int PREG_SPLIT_NO_EMPTY = 0;
const int PREG_SPLIT_DELIM_CAPTURE = 0;
const int PREG_SPLIT_OFFSET_CAPTURE = 0;
const int PREG_NO_ERROR = 0;
const int PREG_INTERNAL_ERROR = 0;
const int PREG_BACKTRACK_LIMIT_ERROR = 0;
const int PREG_RECURSION_LIMIT_ERROR = 0;
const int PREG_BAD_UTF8_ERROR = 0;
const int PREG_BAD_UTF8_OFFSET_ERROR = 0;
const int PREG_BAD_REGEX_ERROR = 0;
const int PREG_GREP_INVERT = 0;
const string PCRE_VERSION = '';
