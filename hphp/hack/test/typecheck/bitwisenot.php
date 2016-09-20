<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function int_not(int $a): int {
  return ~$a;
}

function generic_int_not<T as int>(T $a): int {
  return ~$a;
}

enum Colour : int as int {
  Red = 0;
  Blue = 1;
}

function enum_not(Colour $a): int {
  return ~$a;
}
