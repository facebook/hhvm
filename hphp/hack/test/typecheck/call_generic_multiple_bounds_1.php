<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test<T as (function(): arraykey) as (function(): num)>(T $f): int {
  return $f();
}
