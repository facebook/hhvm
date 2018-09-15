<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function foo<T1, T2>(T1 $x) : T1 {
  return $x;
}

function bar() : int {
  return foo<int, string>(5);
}
