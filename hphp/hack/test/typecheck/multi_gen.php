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

class A<T> {}

function foo<T, T2 as A<T>>(T2 $x): void {
}

function test(A<int> $a): void {
  foo($a);
}
