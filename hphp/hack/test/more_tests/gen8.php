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


function foo1<T>(T $x): T {
  return $x;
}

function foo2<T2>(T2 $x): T2 {
  return $x;
}

function foo3(int $x): int { return 0; }

function test<T3>(bool $b): (function(T3): T3) {
  $x = $b? fun('foo1'): fun('foo2');
  return $x;
}
