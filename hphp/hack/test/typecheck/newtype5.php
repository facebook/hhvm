//// file1.php
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

newtype a<T> as T = T;

//// file2.php
<?hh

function test(a<int> $x): void {}
function test2(a<bool> $x): void {
  test($x);
}
