<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expectNullableInt(?int $_):void { }
function expectSDshape(supportdyn<shape('a' => int, ...)> $s):int {
  $x = $s['a'];
  $y = Shapes::idx($s, 'a');
  expectNullableInt($y);
  return $x;
}
function test2(shape('a' => int) $s):void {
  expectSDshape($s);
}
