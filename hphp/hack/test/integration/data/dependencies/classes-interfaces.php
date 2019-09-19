<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface BaseInterface {}

interface DerivedInterface extends BaseInterface {
  public function routine(): void;
}

function with_interface(DerivedInterface $arg): void {
  $arg->routine();
}

interface SimpleInterface {
  require extends AbstractBase;
}

trait RequiringTrait {
  require implements BaseInterface;
  require implements SimpleInterface;
}

class Implementing extends AbstractBase implements DerivedInterface {
  public function routine(): void {
    $this->must_implement();
  }
  protected function must_implement(): void {}
}

class DerivedImplementing extends Implementing implements SimpleInterface {
  use RequiringTrait;
}

function with_requiring_trait(DerivedImplementing $arg): void {}
