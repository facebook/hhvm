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

function f(): void {
  $x = dict[];
  foreach (vec[1, 2, 3] as $y) {
    $z = $x['test'];
    if (true) {
      $x = null;
      break;
    }
  }
}
