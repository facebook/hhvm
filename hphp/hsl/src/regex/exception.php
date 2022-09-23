<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Regex;

use namespace HH\Lib\Str;

final class Exception extends \Exception {
  public function __construct(Pattern<Match> $pattern, int $code)[] {
    $errors = dict[
      \PREG_INTERNAL_ERROR => 'Internal error',
      \PREG_BACKTRACK_LIMIT_ERROR => 'Backtrack limit error',
      \PREG_RECURSION_LIMIT_ERROR => 'Recursion limit error',
      \PREG_BAD_UTF8_ERROR => 'Bad UTF8 error',
      \PREG_BAD_UTF8_OFFSET_ERROR => 'Bad UTF8 offset error',
      \PREG_BAD_REGEX_ERROR => 'Regex failed to compile',
    ];
    parent::__construct(
      Str\format(
        "%s: %s",
        idx($errors, $code, 'Unknown regex error'),
        \HH\FIXME\UNSAFE_CAST<\HH\Lib\Regex\Pattern<shape(...)>, string>(
          $pattern,
          'Until we have a to_string() function',
        ),
      ),
    );
  }
}
