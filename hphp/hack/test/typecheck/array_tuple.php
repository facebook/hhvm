<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function test(): void {
  $array = array(array('', 0), array('', 0));

  foreach ($array as $update) {
    $index = 1;
    $update[$index] = 0; // force conversion from tuple-like to vec-like
    list($name, $value) = $update;
    hh_show($name);
    hh_show($value);
  }
}
