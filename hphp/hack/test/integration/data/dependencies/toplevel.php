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

function shallow_toplevel(C $c): void  {
  g();
}

function with_generics<Tfirst, Tsecond>(D<Tfirst, Tsecond> $d, E<Tfirst> $e): int {
  return generic<C>();
}

function with_typedefs(Complex $c, shape('x' => int, 'y' => C) $pair) : Point {
  return shape('x' => $pair['x'], 'y' => $c['first']);
}

function with_defaults(int $arg = 42, float $argf = 4.2): void {
}

function call_defaulted(int $arg): void {
  with_defaults($arg);
  with_defaults();
}

function nonexistent_dependency(BogusType $arg): void {}

function builtin_argument_types(Exception $e, keyset<string> $k): void {}
