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
  public function __construct(int $num) {
    $this->result = $num;
  }

  public function overridden(): int {
    return $this->result;
  }

  private int $result;
}

function with_overriding(Derived $arg): int {
  $arg->inherited();
  return $arg->overridden();
}

function call_constructors(): void {
  $a = new ImplementingBase();
  $b = new Derived(0);
}

function variadic(inout int $arg, int ...$args): void{}

function with_nontrivial_fun_decls(): void {
  $num = 17;
  variadic(inout $num, 18, 19);
  $d = new Derived($num);
}

class WithProperties {
  public function __construct(int $arg) {
    $this->first = $arg;
  }

  public int $first;
  public int $second = 0;
}

function use_properties(WithProperties $arg): int {
  return $arg->first + $arg->second;
}

class SimpleClass {
  public function simple_method(): void {}
  public function another_method(): void {
    $this->coarse_grained_dependency();
  }
  public function coarse_grained_dependency(): void {}
}
