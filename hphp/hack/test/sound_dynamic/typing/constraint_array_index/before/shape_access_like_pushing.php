<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

type TShape = supportdyn<shape(
  'a' => string,
  ...
)>;

function test(
  ~TShape $s,
): ~string {
  $y = $s['a'];
  return $y;
}
