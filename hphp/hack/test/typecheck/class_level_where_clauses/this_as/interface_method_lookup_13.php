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

interface J {}

class C {
  static public function foo(): num { return 0.0; }
}

class D extends C implements J {}

interface I where this as C this as J {}

class E extends D implements I {
  static public function foo(): int { return 0; }
}

function test1(E $x): int {
  return $x::foo();  // E::foo
}

function test2(D $x): int {
  return $x::foo();  // Error! C::foo
}

function test3(I $x): int {
  return $x::foo();  // Error! C::foo
}

function test4(I $x): num {
  return $x::foo();  // C::foo
}

function test5(J $x): mixed {
  return $x::foo();  // Error! J::foo not found
}

function test6(C $x): int {
  return $x::foo();  // Error! C::foo
}
