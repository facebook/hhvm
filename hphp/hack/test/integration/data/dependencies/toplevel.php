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

function recursive_function(int $n): int {
  if ($n <= 0) {
    return 0;
  }
  return $n + recursive_function($n - 1);
}

class WithRecursiveMethods {
  public function recursive_instance(): void {
    $this->recursive_instance();
  }
  public static function recursive_static(): void {
    WithRecursiveMethods::recursive_static();
  }
}

function with_mapped_namespace(): void {
  PHP\ini_set('foo', 'bar');
}

function with_built_in_constant(): int {
  return PHP_INT_MAX;
}
