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
  $t = Pair {1, 'foo'};
  return $t[0];
}
function g(): string {
  $t = Pair {1, 'foo'};
  return $t[1];
}


