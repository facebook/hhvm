<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test<T as (function(int): void) as (function(float): void)>(num $x, T $f): string {
  return $f($x);
}
