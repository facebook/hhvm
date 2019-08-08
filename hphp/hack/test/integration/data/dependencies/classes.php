<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class AbstractBase {
  const BASE_CONSTANT = 42;
  public static int $static_base_property = 0;
  public float $base_property = 42.;

  abstract protected function must_implement(): void;
}

function with_abstract(AbstractBase $arg) : float {
  return AbstractBase::BASE_CONSTANT + $arg->base_property +
  AbstractBase::$static_base_property;
}

class ImplementingBase extends AbstractBase {
  public function inherited(): void {}
  public function overridden(): int {
    return -1;
  }

  protected function must_implement(): void {
    $this->inherited();
  }
}

final class Derived extends ImplementingBase {
  public function overridden(): int {
    return 0;
  }
}

function with_overriding(Derived $arg): int {
  $arg->inherited();
  return $arg->overridden();
}
