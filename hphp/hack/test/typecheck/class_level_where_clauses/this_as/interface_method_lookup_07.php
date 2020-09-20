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
  public function foo(): num { return 0.0; }
}

interface I where this as C {}

class D extends C implements I {
  public function foo(): int { return 0; }
}

function test1(D $x): int {
  return $x->foo();  // D::foo
}

function test2(I $x): int {
  return $x->foo();  // Error! I::foo
}

function test3(I $x): num {
  return $x->foo();  // C::foo
}

function test4(C $x): int {
  return $x->foo();  // Error! C::foo
}

function test5(C $x): num {
  return $x->foo();  // C::foo
}
