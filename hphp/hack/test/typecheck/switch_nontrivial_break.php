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
  switch (1) {
    case 1:
      $x = false;
      if (true) {
        break;
      } else {
        break;
      }
    default:
      $x = 1;
  }

  return $x;
}
