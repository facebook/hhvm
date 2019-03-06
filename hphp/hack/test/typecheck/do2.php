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

function test(): int {
  $x = null;
  do {
    for ($i = 0; $i < 10; ++$i) {
      continue;
    }
    $i = 0;
    while ($i++ < 10) {
      continue;
    }
    do {
      continue;
    } while ($i-- > 0);
    $x = 0;
  } while (true);
  return $x;
}
