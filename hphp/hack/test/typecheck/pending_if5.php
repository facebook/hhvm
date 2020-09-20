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

function f(bool $b, bool $c): int {
  if ($b) {
    $x = 1;
  } else if ($c) {
    $x = 2;
  } else {
    $x = false;
  }

  return $x;
}
