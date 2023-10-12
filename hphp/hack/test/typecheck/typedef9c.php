////file1.php
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

type A = A;

////file2.php
<?hh
type B = A;

////file3.php
<?hh
function foo(B $a):void { }
