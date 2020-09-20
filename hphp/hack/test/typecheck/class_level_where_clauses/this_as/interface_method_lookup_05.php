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

class C {}

interface I where this as C {}

class D extends C implements I {
  static public function foo(): int { return 0; }
}

function test1(D $x): int {
  return $x::foo();  // D::foo
}

function test2(D $x): num {
  return $x::foo();  // D::foo
}

function test3(D $x): mixed {
  return $x::foo();  // D::foo
}

function test4(I $x): mixed {
  return $x::foo();  // Error! I::foo & C:foo
}

function test7(C $x): mixed {
  return $x::foo();  // Error! C::foo
}

function test10(): int {
  return D::foo();  // D::foo
}

function test11(): num {
  return D::foo();  // D::foo
}

function test12(): mixed {
  return D::foo();  // D::foo
}

function test14(): mixed {
  return I::foo();  // Error! I::foo & C::foo
}

function test16(): mixed {
  return C::foo();  // Error! C::foo
}
