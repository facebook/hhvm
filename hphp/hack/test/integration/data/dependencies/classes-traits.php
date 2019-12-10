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
