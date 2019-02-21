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
  $x = 0;
  if(true) {
    throw new Exception('');
  }
  else {
    throw new Exception('');
  }
  return $x;
}
