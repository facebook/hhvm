<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function Foo<Tv as Tu, Tu as Tv>(Tv $x): int {
  $y = $x->Foo();
  return $y;
}
