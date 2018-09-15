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

function sum(): int {
  $v = Vector { 1, 2, 3, 4, 5 };
  $s = 0;
  foreach ($v as $k) {
    $s += $k;
  }
  return $s;
}
