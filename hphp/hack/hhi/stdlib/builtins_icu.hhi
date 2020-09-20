<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int UREGEX_CASE_INSENSITIVE = 0;
const int UREGEX_COMMENTS = 0;
const int UREGEX_DOTALL = 0;
const int UREGEX_MULTILINE = 0;
const int UREGEX_UWORD = 0;
const int UREGEX_OFFSET_CAPTURE = 0;

<<__PHPStdLib>>
function icu_match(string $pattern, string $subject, int $flags = 0);
<<__PHPStdLib>>
function icu_match_with_matches(string $pattern, string $subject, inout $matches,
                                int $flags = 0);
<<__PHPStdLib>>
function icu_transliterate(string $str, bool $remove_accents);
<<__PHPStdLib>>
function icu_tokenize(string $text);
