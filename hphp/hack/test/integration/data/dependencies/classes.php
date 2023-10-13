<?hh
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

  <<__Override>>
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

function only_variadic(int ...$args): void {}

function variadic(inout int $arg, int ...$args): void{}

function with_nontrivial_fun_decls(): void {
  $num = 17;
  variadic(inout $num, 18, 19);
  only_variadic($num, 18, 19);
  $d = new Derived($num);
}

class WithProperties {
  public function __construct(int $arg) {
    $this->first = $arg;
  }

  public int $first;
  public int $second = 42;
  public static int $third = 7;
}

function use_properties(WithProperties $arg): int {
  return $arg->first + $arg->second + WithProperties::$third;
}

class SimpleClass {
  public function __construct(string $s, int $i) {}
  public function simple_method(): void {}
  public function another_method(): void {
    $this->coarse_grained_dependency();
  }
  public function coarse_grained_dependency(): void {}
}

class SimpleDerived extends SimpleClass {
  public function __construct(float $f, bool $b, mixed ...$args) {
    parent::__construct('mumble', 42);
  }
  public function call_parent_method(): void {
    parent::simple_method();
    ++SimpleDerived::$calls;
  }

  private static int $calls = 0;
}

interface IWithRequirement {
  require extends A;
}

function with_requiring_interface(IWithRequirement $arg): void {}

function WithNameMatchingClassName(): WithNameMatchingClassName {
  return new WithNameMatchingClassName();
}

class WithNameMatchingClassName {}

function with_classname(): string {
  return SimpleClass::class;
}

function with_parent_constructor_call(): void {
  $_ = new SimpleClass('frob', 1337);
  $_ = new SimpleDerived(3.14, true, null);
}

class WithReactiveMethods {
  public function reactive(): void {}

  public function call_reactive(): void {
    $this->reactive();
  }
}

class WithLateInit {
  <<__LateInit>>
  private int $count;

  public function getCount(): int {
    return $this->count;
  }

  public function setCount(int $count): void {
    $this->count = $count;
  }
}

abstract class TestExtractConstruct {
    public function __construct() {}
}

interface IAsConstraint{}
abstract class WithTparamConstraint<T as IAsConstraint>{}

class WithPropInConstruct<T> {
  public function __construct(public T $x)[] {}
}

class WithTypeConstant {
  const type T = string;
}

class WithTypeConstantParamConstraint<T as WithTypeConstant::T> {
  public function foo(): void {}
}
