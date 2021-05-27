<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Regex;

use namespace HH\Lib\Str;

final class Exception extends \Exception {
  public function __construct(Pattern<Match> $pattern, int $code)[]: void {
    $errors = dict[
      \PREG_INTERNAL_ERROR => 'Internal error',
      \PREG_BACKTRACK_LIMIT_ERROR => 'Backtrack limit error',
      \PREG_RECURSION_LIMIT_ERROR => 'Recursion limit error',
      \PREG_BAD_UTF8_ERROR => 'Bad UTF8 error',
      \PREG_BAD_UTF8_OFFSET_ERROR => 'Bad UTF8 offset error',
    ];
    parent::__construct(
      Str\format(
        "%s: %s",
        idx($errors, $code, 'Invalid pattern'),
        /* HH_FIXME[4110] Until we have a to_string() function */
        $pattern,
      ),
    );
  }
}
