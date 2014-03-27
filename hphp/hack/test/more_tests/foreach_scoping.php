<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function test_func(): int {
  $var = null;
  foreach (array() as $_) {
    $var = 1;
  }
  return $var; // this should be type `int & ?_`, but hack thinks it's `int`
}
