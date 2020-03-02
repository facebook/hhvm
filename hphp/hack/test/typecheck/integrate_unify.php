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

function f() {
  $arr = darray[];
  foreach (varray[] as $x) {
    if (true) {
      foreach (varray[] as $y) {
      }

      continue;
    }
  }

  yield wait_forvar($arr);
}

function wait_forvar(array<int, int> $arr) {}
