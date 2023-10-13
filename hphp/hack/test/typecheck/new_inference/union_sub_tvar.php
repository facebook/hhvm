<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(bool $b): arraykey {
  $x = Vector{}; // new tvar v, $x : Vector<v>
  $y = ($b ? 0 : ($b ? "b" : $x[0])); // $y : int | string | v
  $x[] = $y; // (int | string | v) <: v, should be simplified to int | string <: v
  $z = $x[0];
  return $z;
}
