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
  $b = true;
  $c = false;
  if ($b) {
    $x = 1;
  } else if ($c) {
    $x = false;
  } else {
    $x = 4;
  }

  return $x;
}
