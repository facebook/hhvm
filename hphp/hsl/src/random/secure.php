<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\SecureRandom;

use namespace HH\Lib\{_Private, Math, Str};

/**
 * Returns a cryptographically secure random float in the range [0.0, 1.0)
 * (i.e. the return value is >= 0.0 and < 1.0).
 *
 * For pseudorandom floats, see `PseudoRandom\float`.
 */
function float()[defaults]: float {
  return (float)(namespace\int(0, Math\INT53_MAX - 1) / Math\INT53_MAX);
}

/**
 * Returns a cryptographically secure random integer in the range from `$min` to
 * `$max`, inclusive.
 *
 * For pseudorandom integers, see `PseudoRandom\int`.
 */
function int(
  int $min = \PHP_INT_MIN,
  int $max = \PHP_INT_MAX,
)[defaults]: int {
  invariant(
    $min <= $max,
    'Expected $min (%d) to be less than or equal to $max (%d).',
    $min,
    $max,
  );
  return _Private\Native\random_int($min, $max);
}

/**
 * Returns a securely generated random string of length `$length`. The string is
 * composed of characters from `$alphabet` if `$alphabet` is specified.
 *
 * For pseudorandom strings, see `PseudoRandom\string`.
 */
function string(
  int $length,
  ?string $alphabet = null,
)[defaults]: string {
  return _Private\random_string(
    ($length) ==> \random_bytes($length),
    $length,
    $alphabet,
  );
}
