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

function f($r) {
  if ($r) {
    $y = 0;
  }
  $r = 0;
  return $r;
}

function g() {
  $t = g();
  $t = 'd';
}
