<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Base {}
class Inv<Tinv> {}
function ExpectsInvBase(Inv<Base> $ib): void {}
function Test(mixed $m): void {
  if ($m instanceof Inv) {
    ExpectsInvBase($m);
  }
}
