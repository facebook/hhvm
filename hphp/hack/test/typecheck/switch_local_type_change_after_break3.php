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

function f(bool $b): int {
  $x = 1;
  switch (1) {
    default:
      if ($b) {
        hh_show($x);
        $x = "derp";
        hh_show($x);
        break;
      }
  }
  hh_show($x);

  return $x;
}
