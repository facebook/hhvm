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

function f(): int {
  switch (1) {
    case 1:
      $x = false;
      break;
    default:
      $x = 8;
      break;
  }

  return $x;
}
