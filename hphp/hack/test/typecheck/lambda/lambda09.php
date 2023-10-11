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

// bad_style-ish case inside a lambda
function foo(): void {
  $x = true;
  $l = () ==> {
    if ($x) { // capture $x
      $y = 100; // define $y
    }
    return $y; // error out of scope
  };
}
