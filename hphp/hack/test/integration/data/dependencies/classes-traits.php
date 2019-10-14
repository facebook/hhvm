<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait ImplementingAbstractBase {
  protected function must_implement(): void {
  }
}

trait T {
  require extends AbstractBase;

  public function routine(): void {
    $this->must_implement();
  }
}

class TraitBase extends AbstractBase {
  use ImplementingAbstractBase;
  use T;
}

function with_traits(TraitBase $arg) : void {
  $arg->routine();
}

interface IHasFoo {
  abstract const type TFoo;
  public function getDefaultFoo(): this::TFoo;
}

trait THasFoo {
  require implements IHasFoo;

  public function getFoo(): this::TFoo {
    return $this->getDefaultFoo();
  }
}

class IntFooWrapper implements IHasFoo {
  use THasFoo;
  const type TFoo = int;
  public function getDefaultFoo(): this::TFoo {
    return 0;
  }
}

function with_type_const_from_required_interface(
  IntFooWrapper $w,
): int {
  return $w->getFoo();
}
