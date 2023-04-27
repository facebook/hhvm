<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int UREGEX_CASE_INSENSITIVE;
const int UREGEX_COMMENTS;
const int UREGEX_DOTALL;
const int UREGEX_MULTILINE;
const int UREGEX_UWORD;
const int UREGEX_OFFSET_CAPTURE;

<<__PHPStdLib>>
function icu_match(
  string $pattern,
  string $subject,
  int $flags = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function icu_match_with_matches(
  string $pattern,
  string $subject,
  inout HH\FIXME\MISSING_PARAM_TYPE $matches,
  int $flags = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function icu_transliterate(
  string $str,
  bool $remove_accents,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function icu_tokenize(string $text): HH\FIXME\MISSING_RETURN_TYPE;
