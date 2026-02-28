<?hh
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

class ImplementsBuiltin implements StringishObject {
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

<<__ConsistentConstruct>>
interface IWithNullaryConstructor {
  public function __construct();
}

trait TDummy implements IWithNullaryConstructor {}

class WithOptionalConstructorArguments implements IWithNullaryConstructor {
  use TDummy;

  public function __construct(?int $x = null, ?string $y = null) {}

  public static function get(): this {
    return new static();
  }
}

<<__ConsistentConstruct>>
class WithConsistentConstruct {
  public function __construct() {}
}

interface IExtendsWithConsistentConstruct {
  require extends WithConsistentConstruct;
}

trait TExtendsWithConsistentConstruct {
  require implements IExtendsWithConsistentConstruct;

  public static function get(): this {
    return new static();
  }
}

function with_IEWGPCOUP(IEWGPCOUB $x): IEWGP {
  return $x->f();
}

interface IEWGPCOUB extends IEWGPMB, IEPCOUB {}

interface IEWGPMB extends IEPMB {
  abstract const type T as IEWGP;
}

interface IEPCOUB extends IEMBUIDCOUB {}

interface IEPMB extends IEMBUIDMB {
  abstract const type T as IEP;
}

interface IEMBUIDCOUB extends IEMBUIDMB {}

interface IEMBUIDMB {
  abstract const type T as IEMBUID;
  public function f(): this::T;
}

interface IEWGP extends IEP {}

interface IEP extends IEMBUID {}

interface IEMBUID {}

<<__Sealed(OnSealedWhitelist::class)>>
interface WithSealedWhitelist<T as arraykey> {
}

interface OnSealedWhitelist<T as arraykey> extends WithSealedWhitelist<T> {
  public function __construct(T ...$values);
}

function with_arg_with_sealed_whitelist(WithSealedWhitelist<int> $f): void {}
