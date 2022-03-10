<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function foo(
  ~supportdyn<shape('a' => int, ...)> $s,
  ~?int $y = null,
): ~bool {
  $x = $s['a'];
  $y ??= $s['a'];
  return 3 <= $y;
}
