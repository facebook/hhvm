<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class GenericBase<Tfirst, Tsecond> {
  public function __construct(public Tfirst $first, public Tsecond $second): void {}
}

enum Mode: int as int {
  One = 1;
  Two = 2;
}

function with_enum(Mode $arg): int {
  return $arg;
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
