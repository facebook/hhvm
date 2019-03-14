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

function foo(?int $x, int $y): int {
  if ($x === null) {
    switch ($y) {
      case 2:
        return 2;
      case 0:
    }
  }
  return $x;
}
