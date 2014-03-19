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

class X<T> {
  public ?T $x, $y, $z;
}

function test(): int {
  $obj = new X();
  $obj->x = 0;
  $obj->y = 1;
  $obj->z = 2;
  for($i = 0; $i < 3; $i++) {
    $obj->x = $obj->y;
    $obj->y = $obj->z;
    $obj->z = 'hello';
  }
  return $obj->x;
}
