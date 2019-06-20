<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Test {
  abstract const type Ta as num;
  const type Tb = this::Ta;

  const type Tc = int;
  const type Td = this::Tc;

  const type Te as nonnull = arraykey;
  const type Tf = this::Te;
  const type Tg = C;

  public function f(): void {
    3 as this::Tb; // looks safe, but points to abstract type const
    4 as this::Td;
    5 as this::Tf; // same thing, but partially abstract
    6 as this::Tg::T; // make sure this resolves to the C::Tc and not Test::Tc
  }
}

abstract class C {
  abstract const type Tc;
  const type T = this::Tc;
}
