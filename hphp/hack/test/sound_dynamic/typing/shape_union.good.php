<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function expectSD(supportdyn<mixed> $m):void { }
function test1(supportdyn<shape(?'a' => int, ...)> $s, supportdyn<shape('b' => bool, ...)> $t, bool $b):void  {
  if ($b) $s = $t;
  $x = Shapes::idx($s,'a');
  expectSD($x);
}
function test2(shape(?'a' => int) $s, supportdyn<shape('b' => bool, ...)> $t, bool $b):void  {
  if ($b) $s = $t;
  $x = Shapes::idx($s,'a');
  expectSD($x);
}
function test3(shape(?'a' => int) $s, supportdyn<shape('b' => bool, ...)> $t, bool $b):void  {
  if ($b) $t = $s;
  $x = Shapes::idx($t,'a');
  expectSD($x);
}
