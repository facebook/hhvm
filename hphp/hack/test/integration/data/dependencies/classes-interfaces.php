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

class ImplementsBuiltin implements Stringish {
  public function __toString(): string {
    return "";
  }
}

function does_not_use_class_methods(ImplementsBuiltin $arg): void {}

abstract class Bee {
  public function f(): void {}
}

interface Eye {
  require extends Bee;
}

interface Jay extends Eye {}

function with_indirect_require_extends(Jay $x): void {
  $x->f();
}
