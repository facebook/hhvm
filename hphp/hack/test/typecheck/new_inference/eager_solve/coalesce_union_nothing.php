<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():bool { return false; }
}
function vecmap<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  return vec[];
}

function testit(?vec<C> $v):void {
  $w = $v ?? vec [];
  list($x,$y) = $w;
  $w = vecmap($w, $x ==> $x->foo());
}
