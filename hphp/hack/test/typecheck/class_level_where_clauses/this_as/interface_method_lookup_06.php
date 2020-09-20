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

class C {
  static public function foo(): num { return 0.0; }
}

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

function test4(I $x): int {
  return $x::foo();  // Error! I::foo & C::foo = C::foo
}

function test5(I $x): num {
  return $x::foo();
}

function test6(I $x): mixed {
  return $x::foo();
}

function test7(C $x): int {
  return $x::foo();  // Error! C::foo
}

function test8(C $x): num {
  return $x::foo();  // C::foo
}

function test9(C $x): mixed {
  return $x::foo();  // C::foo
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

function test13(I $x): int {
  return I::foo();  // Error! I::foo & C::foo = C::foo
}

function test14(I $x): num {
  return I::foo();  // I::foo & C::foo = C::foo
}

function test15(I $x): mixed {
  return I::foo();  // I::foo & C::f00 = C::foo
}

function test16(): int {
  return C::foo();  // Error! C::foo
}

function test17(): num {
  return C::foo();  // C::foo
}

function test18(): mixed {
  return C::foo();  // C::foo
}
