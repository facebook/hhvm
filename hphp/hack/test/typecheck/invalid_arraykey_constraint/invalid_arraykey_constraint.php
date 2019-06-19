<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// IAC = invalid arraykey constraint

type int_alias = int;
type string_alias = string;
type float_alias = float;
type null_alias = null;

abstract class TestGoodClass<Ta as string> {
  abstract const type Tb as arraykey;
  public darray<Ta, int> $a = darray[];
  public darray<this::Tb, int> $b = darray[];
}

abstract class TestBadClass<Ta as num, Tb> {
  abstract const type Tc as num;
  abstract const type Td;
  public darray<Ta, int> $a = darray[];
  public darray<Tb, int> $b = darray[];
  public darray<this::Tc, int> $c = darray[];
  public darray<this::Td, int> $d = darray[];
}

function good(
  darray<int, int> $_,
  darray<string, int> $_,
  darray<int_alias, int> $_,
  darray<string_alias, int> $_,
): void {}

function bad(
  darray<float, int> $_,
  darray<null, int> $_,
  darray<float_alias, int> $_,
  darray<null_alias, int> $_,
): void {}
