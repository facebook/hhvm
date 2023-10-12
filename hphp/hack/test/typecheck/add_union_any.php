<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

/* HH_FIXME[4030] */
function makeAny() {
  return 3;
}

function expectInt(int $x):void { }

function testit(bool $f, int $z):void {
  if ($f) $z = makeAny();
  $w = $z + 1;
  expectInt($w);
}
