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

class A {}

function myFunc(): A {
  $var = null;

  // Hack assumes that this loop might or might not run.
  for ($i = 0; $i < 1; $i++) {
    $var = new A();
  }
  return $var;
}
