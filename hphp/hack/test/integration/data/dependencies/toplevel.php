<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I1 {}

class A0 {}

class A extends A0 implements I1 {}

class B implements I1 {}

class C extends A {}

class D<Tfirst, Tsecond> extends B {}

class E<T> extends D<T, int> {}

function generic<T>(): int {
  return 1;
}

function g() : void {
  $b = new B();
}

function f(C $c): void  {
  g();
}

function h<Tfirst, Tsecond>(D<Tfirst, Tsecond> $d, E<Tfirst> $e): int {
  return generic<C>();
}
