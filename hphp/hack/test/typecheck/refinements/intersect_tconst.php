<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface J1 { }
interface J2 { }
interface I1 {
  abstract const type T1 as J1;
  abstract const type T2;
  public function getID(): (this::T1,this::T2);
}

interface I2 {
  abstract const type T2 as J2;
}

function expectPair((J1,J2) $x):void { }
function test1(I1 $x):void {
  if ($x is I2) {
    $y = $x->getID();
    expectPair($y);
  }
}
function test2(I2 $x):void {
  if ($x is I1) {
    // So this will get instantiated to (I2 & I1)
    // We then attempt this::T1 and this::T2
    $y = $x->getID();
    expectPair($y);
  }
}
