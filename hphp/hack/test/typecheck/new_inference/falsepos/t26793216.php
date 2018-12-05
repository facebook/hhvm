<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function g<T1 as int>(?T1 $x): void {}

function test<T2 as ?int>(T2 $x): void {
  g($x);
}
