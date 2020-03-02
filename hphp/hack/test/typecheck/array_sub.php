<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function get_keys<T1, T2>(array<T1, T2> $x): array<T1> {
  $result = varray[];
  foreach ($x as $k => $v) {
    $result[] = $k;
  }
  return $result;
}

function test(array<int> $a): void {
  get_keys($a);
}
