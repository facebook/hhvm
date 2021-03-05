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

function foo(?int $x, int $y): int {
  if ($x === null) {
    switch ($y) {
      case 2:
        return 2;
      case 0:
        // this will not fallthrough in the implicit
        //   default:
        //       throw new RuntimeException('non-exhaustive switch')
        // instead, it falls through right out of the switch
        // statment
        //
        // for better or worse...
    }
  }
  return $x;
}
