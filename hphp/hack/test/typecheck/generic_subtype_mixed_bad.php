<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test<T1, T2 super int>(T1 $x): ?T2 {
  return $x;
}
