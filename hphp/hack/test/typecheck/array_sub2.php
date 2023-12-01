<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function get_keys<T2>(darray<bool, T2> $x): varray<bool> {
  $result = vec[];
  foreach ($x as $k => $v) {
    $result[] = $k;
  }
  return $result;
}

function test(varray<int> $a): void {
  get_keys($a);
}
