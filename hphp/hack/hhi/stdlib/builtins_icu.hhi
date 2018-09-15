<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

define('UREGEX_CASE_INSENSITIVE', 0);
define('UREGEX_COMMENTS', 0);
define('UREGEX_DOTALL', 0);
define('UREGEX_MULTILINE', 0);
define('UREGEX_UWORD', 0);
define('UREGEX_OFFSET_CAPTURE', 0);
<<__PHPStdLib>>
function icu_match($pattern, $subject, &$matches = null, $flags = 0) { }
<<__PHPStdLib>>
function icu_transliterate($str, $remove_accents) { }
<<__PHPStdLib>>
function icu_tokenize($text) { }
