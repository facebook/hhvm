//// file1.php

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

class Duck {}
class Y {}

newtype opaque_duck as Y = Duck;

//// file2.php

<?hh // strict

function test(opaque_duck $x): Duck {
  return $x;
}
