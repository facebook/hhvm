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

function expect_arraykey(arraykey $ak):void { }
function test(): void {
  $array = vec[vec['', 0], vec['', 0]];

  foreach ($array as $update) {
    $index = 1;
    $update[$index] = 0; // force conversion from tuple-like to vec-like
    list($name, $value) = $update;
    expect_arraykey($name);
    expect_arraykey($value);
  }
}
