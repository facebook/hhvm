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
function preg_grep($pattern, $input, $flags = 0) { }
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
function preg_match($pattern, $subject, &$matches = array(), $flags = 0,
                    $offset = 0): int { }
<<__PHPStdLib>>
function preg_match_all($pattern, $subject, &$matches = array(), $flags = 0,
                        $offset = 0) { }
<<__PHPStdLib>>
function preg_replace($pattern, $replacement, $subject, $limit = -1,
                      &$count = null) { }
<<__PHPStdLib>>
function preg_replace_callback($pattern, $callback, $subject, $limit = -1,
                               &$count = null) { }
<<__PHPStdLib>>
function preg_replace_callback_array($patterns_and_callbacks, $subject,
                                     $limit = -1, &$count = null) { }
<<__PHPStdLib>>
function preg_split($pattern, $subject, $limit = -1, $flags = 0) { }
<<__PHPStdLib, __Rx>>
function preg_quote($str, $delimiter = null) { }
<<__PHPStdLib>>
function preg_last_error() { }
<<__PHPStdLib>>
function ereg_replace($pattern, $replacement, $str) { }
<<__PHPStdLib>>
function eregi_replace($pattern, $replacement, $str) { }
<<__PHPStdLib>>
function ereg($pattern, $str, &$regs = null) { }
<<__PHPStdLib>>
function eregi($pattern, $str, &$regs = null) { }
<<__Deprecated('Use explode() or preg_split().'), __PHPStdLib>>
function split($pattern, $str, $limit = -1) { }
<<__PHPStdLib>>
function spliti($pattern, $str, $limit = -1) { }
<<__PHPStdLib>>
function sql_regcase($str) { }

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
const int PREG_GREP_INVERT = 0;
const string PCRE_VERSION = '';
