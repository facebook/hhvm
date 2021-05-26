<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function f(): void {
  $dec = 9223372036854775808;
  $hex = 0x8000000000000000;
  $bin = 0b1000000000000000000000000000000000000000000000000000000000000000;
  $oct = 01000000000000000000000;

  $oct = 018;
  $bin = 0b12;
}
