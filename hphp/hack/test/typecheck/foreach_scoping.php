<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function test_func(): int {
  $var = null;
  foreach (varray[] as $_) {
    $var = 1;
  }
  return $var; // this should be type `int & ?_`, but hack thinks it's `int`
}
