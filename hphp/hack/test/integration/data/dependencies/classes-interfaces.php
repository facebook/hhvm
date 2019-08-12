<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface BaseInterface {}

interface DerivedInterface extends BaseInterface {}

function with_interface(DerivedInterface $arg): void {}

interface SimpleInterface {
  require extends AbstractBase;
}

trait RequiringTrait {
  require implements BaseInterface;
  require implements SimpleInterface;
}

class Implementing extends AbstractBase implements DerivedInterface {
  protected function must_implement(): void {}
}

class DerivedImplementing extends Implementing implements SimpleInterface {
  use RequiringTrait;
}

function with_requiring_trait(DerivedImplementing $arg): void {}
