<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function len(string $s): int {
  return 0;
}
function apply_curried((function(int): (function(string): bool)) $f): bool {
  return ($f)(3)("a");
}
function apply_curried_generic<T1, T2, T3>(
  (function(T1): (function(T2): T3)) $f,
  T1 $x1,
  T2 $x2,
): T3 {
  return ($f)($x1)($x2);
}
function test_curried(): void {
  $b = apply_curried($i ==> $s ==> $i < 3 + len($s));
  $r = apply_curried_generic((int $i) ==> $s ==> $i < 3 + len($s), 3, "a");
}
