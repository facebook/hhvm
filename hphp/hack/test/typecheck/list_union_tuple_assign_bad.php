<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
function foo(bool $b): int {
  if ($b) {
    $a = tuple(42, 1);
  } else {
    $a = tuple('bar', 2);
  }
  list($x, $y) = $a;
  return $x;
}
