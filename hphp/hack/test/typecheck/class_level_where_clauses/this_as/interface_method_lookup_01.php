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
  public function foo(): C { return $this; }
}

interface I where this as C {
  public function foo(): I;
}

class D extends C implements I {
  public function foo(): D { return $this; }
}

function test1(D $x): D {
  return $x->foo();  // D::foo
}

function test2(D $x): I {
  return $x->foo();  // D::foo
}

function test3(D $x): C {
  return $x->foo();  // D::foo
}

function test4(I $x): D {
  return $x->foo();  // Error! I::foo
}

function test5(I $x): I {
  return $x->foo();  // I::foo
}

function test6(I $x): C {
  return $x->foo();  // I::foo
}

function test7(C $x): D {
  return $x->foo();  // Error! C::foo
}

function test8(C $x): I {
  return $x->foo();  // Error! C::foo
}

function test9(C $x): C {
  return $x->foo();  // C::foo
}
