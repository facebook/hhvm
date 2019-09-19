<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public function foo():int { return 3; }
}

/* HH_FIXME[4030] */
function makeAny() {
  return 3;
}

function testit(?C $co, bool $b, C $c):void {
  $c->foo();
  $z = $co?->foo();
  if ($b) $co = makeAny();
  $z2 = $co?->foo();
  $w = $z . "a";
}
