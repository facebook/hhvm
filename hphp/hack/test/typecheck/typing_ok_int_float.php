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

// checking that an integer can always be used where a float
// is expected

function test2(): void {
  $x = 1 + 1.0;
  $y = 1 / 2 + 1;
}
