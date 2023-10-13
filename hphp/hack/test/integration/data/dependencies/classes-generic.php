<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class GenericBase<Tfirst, Tsecond> {
  const int GENERIC_CONSTANT = -(1 + 2);
  public function __construct(public Tfirst $first, public Tsecond $second) {}
}

type GenericType<T> = GenericBase<T, int>;

enum Mode: int as int {
  One = 1;
  Two = 2;
}

function with_enum_and_constant(Mode $arg): int {
  return $arg + Mode::One + GenericBase::GENERIC_CONSTANT;
}

class GenericDerived<Tfirst> extends GenericBase<Tfirst, Mode> {
  public function __construct(Tfirst $first, Mode $second) {
    parent::__construct($first, $second);
    $this->property = $second;
  }

  protected int $property;

  public function foo(): void {}
}

class First {}
class Second {}

class NonGenericDerived extends GenericBase<First, Second> {}

class Regular {
  public function generic_method<T>(T $arg): void {}
}

function with_generic_method(int $arg): void {
  $r = new Regular();
  $r->generic_method($arg);
}

function with_generic_method_with_wildcard_tparam(int $arg): void {
  $r = new Regular();
  $r->generic_method<_>($arg);
}

function with_properties<T>(GenericDerived<T> $arg) : Mode {
  $x = new GenericDerived<int>(1, Mode::Two);
  return $arg->second;
}

function with_generic_type<T>(GenericType<T> $arg): void {
}

function with_non_generic_type(NonGenericDerived $_): void {}

interface GenericInterface<Tfirst, Tsecond> {}

interface IGenericDerived<T> extends GenericInterface<T, int> {
  require extends GenericBase<float, T>;
}

function with_generic_interface<T>(IGenericDerived<T> $arg): void {}

function with_is_refinement<Tfirst, Tsecond>(
  GenericBase<Tfirst, Tsecond> $x,
): void {
  if ($x is GenericDerived<_>) {
    $x->foo();
  }
}

class BoundedGeneric<T as arraykey> {
  public function emptyKeyset(): keyset<T> {
    return keyset[];
  }
}

function with_bounded_generic_class_tparam(BoundedGeneric<int> $x): keyset<int> {
  return $x->emptyKeyset();
}

interface IResult<+T> {}

class Result<+T> implements IResult<T> {}

interface IKwery<TResult as Result<mixed>> {}

class Kwery<TValue, TResult as Result<TValue>> implements IKwery<TResult> {}

function kwery(): Kwery<int, Result<int>> {
  return new Kwery();
}

<<__ConsistentConstruct>>
abstract class Box<+T> {
  private function __construct(private T $value) {}
  final public static function make(T $value): this {
    return new static($value);
  }
}

class IntBox extends Box<int> {}

function with_contra_tparam(): Box<int> {
  return IntBox::make(42);
}

class WithReifiedGenerics<reify T as arraykey> {}

function with_reified_generics(): WithReifiedGenerics<int> {
  return new WithReifiedGenerics<int>();
}
