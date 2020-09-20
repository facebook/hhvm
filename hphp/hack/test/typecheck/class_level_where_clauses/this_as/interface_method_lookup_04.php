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

interface I where this as C {
  static public function foo(): mixed;
}

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

function test4(I $x): int {
  return $x::foo();  // Error! I::foo
}

function test5(I $x): num {
  return $x::foo();  // Error! I::foo
}

function test6(I $x): mixed {
  return $x::foo();  // I::foo
}

function test7(C $x): int {
  return $x::foo();  // Error! C::foo not found
}

function test8(C $x): num {
  return $x::foo();  // Error! C::foo not found
}

function test9(C $x): mixed {
  return $x::foo();  // Error! C::foo not found
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

function test13(): int {
  return I::foo();  // Error! I::foo can't be called
}

function test14(): num {
  return I::foo();  // Error! I::foo can't be called
}

function test15(): mixed {
  return I::foo();  // Error! I::foo can't be called
}

function test16(): int {
  return C::foo();  // Error! C::foo not found
}

function test17(): num {
  return C::foo();  // Error! C::foo not found
}

function test18(): mixed {
  return C::foo();  // Error! C::foo not found
}
