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

function foo(array<int> $arr): void {
  foreach ($arr as $a) {
    if ($a == 1) {
      $b = 1;
    } else {
      continue;
    }
    echo $b;
  }
}
