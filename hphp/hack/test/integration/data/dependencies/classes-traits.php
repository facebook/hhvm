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

abstract class HasBar {
  abstract const type TBar;
  public function getDefaultBar(): ?this::TBar {
    return null;
  }
}

interface IHasBar {
  const type TBar = string;
}

class StringBarWrapper extends HasBar implements IHasBar {
  public function getBar(): this::TBar {
    $bar = $this->getDefaultBar();
    return $bar ?? 'bar';
  }
}

function with_type_const_from_implemented_interface(
  StringBarWrapper $w,
): string {
  return $w->getBar();
}

interface IHasBaz {
  abstract const type TBaz as IHasQuux;
  const type TQuux = this::TBaz::TQuux;
  public function takeQuux(this::TQuux $_): void;
}

interface IHasQuux {
  abstract const type TQuux;
}

interface IntBazWrapper extends IHasBaz {
  const type TBaz = IntQuuxWrapper;
}

class IntQuuxWrapper implements IHasQuux {
  const type TQuux = int;
}

function with_nested_type_const(IntBazWrapper $x): void {
  $x->takeQuux(42);
}

interface IContext {}

interface IUniverse {
  abstract const type TContext as IContext;
}

class Universe implements IUniverse {
  const type TContext = IContext;
}

interface ICreation {
  abstract const type TUniverse as IUniverse;
  const type TContext = this::TUniverse::TContext;
}

abstract class BaseQuery<TContext> {}

abstract class Query extends BaseQuery<this::TContext> {
  abstract const type TCreation as ICreation;
  const type TContext = this::TCreation::TContext;
}

class Frob implements ICreation {
  const type TUniverse = Universe;
}

trait TFrobQuery {
  require extends BaseQuery<Frob::TContext>;
}

final class FrobQuery extends Query {
  use TFrobQuery;
  const type TCreation = Frob;
}

function frob_query(): FrobQuery {
  return new FrobQuery();
}

interface ICorge {
  abstract const type Tthis as this;
  public function get(): this::Tthis;
}

final class Corge implements ICorge {
  const type Tthis = Corge;
  public function get(): this::Tthis {
    return $this;
  }
}

function corge(Corge $x, ICorge $y): void {
  $_ = $x->get();
  $_ = $y->get();
}

interface IWibble {
  public function f(): void;
}

trait TWibble {
  require implements IWibble;
  public function f(): void {}
}

class Wibble implements IWibble {
  use TWibble;
}

function with_method_defined_in_trait(IWibble $w, Wibble $_): void {
  $w->f();
}

abstract class WobbleBase {
  abstract public function f(): void;
}

trait TWobble {
  require extends WobbleBase;
  public function f(): void {}
}

class Wobble extends WobbleBase {
  use TWobble;
}

function with_method_defined_in_trait2(WobbleBase $w, Wobble $_): void {
  $w->f();
}

abstract class AAA {
  abstract const type T;
}

abstract class BBB {
  abstract const type TA as AAA;
}

abstract class CCC {
  abstract const type TB as BBB;
  const type TA = this::TB::TA;
  const type T = this::TA::T;

  public function with_nested_type_access(this::T $_): void {}
}

interface IFlob {
  abstract const type T;
  public function f1(): void;
  public static function f2(): this::T;
}

interface IToto {
  const type T = int;
}

abstract class FlobBase {
  public function f1(): void {}
  public static function f2(): int {
    return 42;
  }
}

final class Flob extends FlobBase implements IFlob, IToto {
  use TFlob;
}

trait TFlob {
  require implements IFlob;

  public function g(Flob $_): void {
    $this->f1();
    $_ = static::f2();
  }
}

final class Flobby extends FlobBase {
  use TFlobby;
}

trait TFlobby {
  require extends FlobBase;

  final public function g(): void {
    $flobby = $this->asFlobby();
    $flobby->f1();
  }

  private function asFlobby(): Flobby {
    return $this as Flobby;
  }
}

<<__Sealed(Mumble::class)>>
interface SealedInterface {
  public function method(): void;
  public function otherMethod(): void;
}

<<__Sealed(Mumble::class)>>
trait SealedTrait {
  require implements SealedInterface;
  public function method(): void {}
  public function otherMethod(): void {}
}

final class Mumble implements SealedInterface {
  use SealedTrait;
}
