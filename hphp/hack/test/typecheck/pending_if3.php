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
  if (true) {
    $x = false;
  } else if (false) {
    $x = 2;
  } else {
    $x = 4;
  }

  return $x;
}
