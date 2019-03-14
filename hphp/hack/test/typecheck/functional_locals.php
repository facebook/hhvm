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
  if (false) {
    $x = 44;
  }
  if (true) {
    $x = 0;
  } else {
    $x = 1;
  }
  return 0;
}
