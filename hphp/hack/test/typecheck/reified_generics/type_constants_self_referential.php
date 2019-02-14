<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Test {
  abstract const type Ta as num;
  const type Tb = this::Ta;

  const type Tc = int;
  const type Td = this::Tc;

  const type Te as nonnull = arraykey;
  const type Tf = this::Te;

  public function f(): void {
    3 as this::Tb; // looks safe, but points to abstract type const
    4 as this::Td;
    5 as this::Tf; // same thing, but partially abstract
  }
}
