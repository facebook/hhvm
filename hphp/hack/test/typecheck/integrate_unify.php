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

function f() {
  $arr = array();
  foreach (array() as $x) {
    if (true) {
      foreach (array() as $y) {
      }

      continue;
    }
  }

  yield wait_forvar($arr);
}
