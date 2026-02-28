<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\PseudoRandom;

use namespace HH\Lib\{_Private, Math, SecureRandom};

/**
 * Returns a pseudorandom float in the range [0.0, 1.0) (i.e. the return value
 * is >= 0.0 and < 1.0). This is NOT suitable for cryptographic uses.
 *
 * For secure random floats, see `SecureRandom\float`.
 */
function float()[leak_safe]: float {
  return (float)namespace\int(0, Math\INT53_MAX - 1) / Math\INT53_MAX;
}

/**
 * Returns a pseudorandom integer in the range from `$min` to `$max`, inclusive.
 * This is NOT suitable for cryptographic uses.
 *
 * For secure random integers, see `SecureRandom\int`.
 */
function int(
  int $min = \PHP_INT_MIN,
  int $max = \PHP_INT_MAX,
)[leak_safe]: int {
  invariant(
    $min <= $max,
    'Expected $min (%d) to be less than or equal to $max (%d).',
    $min,
    $max,
  );
  return _Private\Native\pseudorandom_int($min, $max);
}

/**
 * Returns a pseudorandom string of length `$length`. The string is composed of
 * characters from `$alphabet` if `$alphabet` is specified. This is NOT suitable
 * for cryptographic uses.
 *
 * For secure random strings, see `SecureRandom\string`.
 */
function string(
  int $length,
  ?string $alphabet = null,
)[leak_safe]: string {
  // This is a temporary alias. You should never, ever depend on this function
  // being cryptographically secure.
  return SecureRandom\string($length, $alphabet);
}
