//// file1.php

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

newtype a as int = int;

//// file2.php

<?hh

function test(a $x): void {}
function test2(int $x): void {
  test($x);
}
