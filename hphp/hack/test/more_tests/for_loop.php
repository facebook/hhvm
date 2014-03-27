<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class A {}

function myFunc(): A {
  $var = null;

  // Hack assumes that this loop might or might not run.
  for ($i = 0; $i < 1; $i++) {
    $var = new A();
  }
  return $var;
}
