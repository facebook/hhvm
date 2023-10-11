<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(bool $b): ?int {
  $x = Vector{}; // new tvar v, $x : Vector<v>
  $y = ($b ? 0 : ($b? null : $x[0])); // $y : ?(int | v)
  $x[] = $y; // ?(int | v) <: v, should be simplified to ?int <: v
  return $x[0];
}
