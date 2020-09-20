<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Lib\_Private\Native {
  function pseudorandom_int(int $min = \PHP_INT_MIN, int $max = \PHP_INT_MAX): int;
  function pseudorandom_seed(int $seed): void;
  function random_int(int $min = \PHP_INT_MIN, int $max = \PHP_INT_MAX): int;
}
