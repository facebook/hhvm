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

class C {};
interface I {};

function f(num $n1, int $n2): num {
  return 1 / $n2;
}

function test(): void {
  $c = new C();
  f($c, 2);
}
