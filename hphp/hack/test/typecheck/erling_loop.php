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

function test(): int {
  $x = 0;
  $y = 1;
  $z = 2;
  for ($i = 0; $i < 3; $i++) {
    $x = $y;
    $y = $z;
    $z = 'hello';
  }
  return $x;
}
