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

// testing that names existing on both sides of if is defined

function stmt(): void {
  switch (true) {
    case true:
      $x = 0;
      break;
    default:
      $x = 1;
      break;
  }
  $x += 1;
}
