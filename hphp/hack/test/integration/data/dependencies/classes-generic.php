<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class GenericBase<Tfirst, Tsecond> {
  const int GENERIC_CONSTANT = -(1 + 2);
  public function __construct(public Tfirst $first, public Tsecond $second) {}
}

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

function with_properties<T>(GenericDerived<T> $arg) : Mode {
  $x = new GenericDerived<int>(1, Mode::Two);
  return $arg->second;
}
