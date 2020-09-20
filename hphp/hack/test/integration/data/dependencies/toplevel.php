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

function generic_with_bound<T as arraykey>(T $x): keyset<T> {
  return keyset[$x];
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

function with_generics_with_bounds(int $x): keyset<int> {
  return generic_with_bound($x);
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

function with_default_and_variadic(mixed $x, ?string $y = null, mixed ...$z): void {}

function call_with_default_and_variadic(string $s): void {
  with_default_and_variadic(42);
  with_default_and_variadic(42, 'meaning of life');
  with_default_and_variadic(42, '%s', $s);
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

<<__Rx>>
function reactive(mixed $x = null): void {}

<<__Rx>>
function call_reactive(): void {
  reactive();
}

<<__RxShallow>>
function shallow_reactive(): void {}

<<__RxShallow>>
function call_shallow_reactive(): void {
  shallow_reactive();
}

class Fred {}

class Thud {
  public int $n;
  public function __construct(Fred $_) {
    $this->n = 42;
  }
}

function with_constructor_dependency(Thud $x): int {
  return $x->n;
}

function with_newtype_with_bound(dict<N, mixed> $_): void {}

newtype M as N = nothing;

function with_newtype_with_newtype_bound(M $_): void {}

type UNSAFE_TYPE_HH_FIXME_<T> = T;

/* HH_FIXME[4101] */
type UNSAFE_TYPE_HH_FIXME = UNSAFE_TYPE_HH_FIXME_;

function with_unsafe_type_hh_fixme(UNSAFE_TYPE_HH_FIXME $x): int {
  return $x;
}

type Option<T> = Id<?T>;
type Id<T> = T;

class WithTypeAliasHint {
  private Option<int> $x = null;

  public function getX(): Option<int> {
    return $this->x;
  }
}
