<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

type FAKE<T> = T;

/* HH_FIXME[4101] Sadly, yes */
type MISSING = FAKE;

function getMissing(): ~MISSING {
  return "A";
}

function expectLikeInt(~int $x):void { }
function testforeach():void {
  $x = getMissing();
  if ($x is nonnull) {
    foreach ($x as $y) {
      expectLikeInt($y);
    }
  }
}
