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

function f(): int {
  $x = 1;
  switch (1) {
    case 1:
      if (true) {
        $x = "derp";
        break;
      }
      $x = 10;
  }

  return $x;
}
