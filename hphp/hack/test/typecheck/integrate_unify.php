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

function f(): Generator<int, int, void> {
  $arr = dict[];
  foreach (vec[] as $x) {
    if (true) {
      foreach (vec[] as $y) {
      }

      continue;
    }
  }

  yield wait_forvar($arr);
}

function wait_forvar(darray<int, int> $arr): int { return $arr[0]; }
