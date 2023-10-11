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
  switch (true) {
  case true:
    $x = $x + 1;
    break;
  default:
    $x = $x + 1;
    break;
  }

  return $x;
}
