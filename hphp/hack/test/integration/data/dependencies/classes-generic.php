<?hh // strict
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
}

class Regular {
  public function generic_method<T>(T $arg): void {}
}

function with_generic_method(int $arg): void {
  $r = new Regular();
  $r->generic_method($arg);
}

function with_properties<T>(GenericDerived<T> $arg) : Mode {
  $x = new GenericDerived<int>(1, Mode::Two);
  return $arg->second;
}

function with_generic_type<T>(GenericType<T> $arg): void {
}
