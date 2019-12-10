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

abstract class BB {
  abstract public function f(): void;
}

interface II {
  abstract const type T;
  public function g(): this::T;
  public function h(): void;
}

final class CC extends BB implements II {
  const type T = int;
  public function f(): void {}
  public function g(): int {
    return 42;
  }
  public function h(): void {}
}

function with_implementations(BB $b, II $i, CC $c): void {
  $b->f();
  $_ = $i->g();
}
