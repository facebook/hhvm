<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I1 {}

class A0 {}

class A extends A0 implements I1 {}

class B implements I1 {}

class C extends A {}

class D<Tfirst, Tsecond> extends B {}

class E<T> extends D<T, int> {}

type Complex = shape('first' => int, 'second' => B);

// TODO: check that the type alias is opaque
newtype Point = shape('x' => int, 'y' => int);

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

function with_types(Complex $c, shape('x' => int, 'y' => C) $pair) : Point {
  return shape('x' => $pair['x'], 'y' => $c['first']);
}
