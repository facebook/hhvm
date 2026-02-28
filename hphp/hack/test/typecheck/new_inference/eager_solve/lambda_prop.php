<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Contra<-T> {
  public function __construct(private T $item) { }
}

function expect<T>(T $x):void { }

class Base {
  public int $fld = 0;
}
class Unrelated {
  public int $fld = 2;
}
class Derived extends Base {
}
function applyAndReturn<T1,T2>(T1 $x, (function(T1):T2) $f):(T2,Contra<T1>) {
  $r = ($f)($x);
  return tuple($r,new Contra($x));
}

function testinherit(Derived $d):void {
  $z1 = applyAndReturn(new Derived(), $y ==> $y->fld);
  expect<Contra<Derived>>($z1[1]);
  $z2 = applyAndReturn(new Derived(), $y ==> $y->fld);
  expect<Contra<Base>>($z2[1]);
  $z3 = applyAndReturn($d, $y ==> $y->fld);
  expect<Contra<Derived>>($z3[1]);
  $z4 = applyAndReturn($d, $y ==> $y->fld);
  expect<Contra<Base>>($z4[1]);
}
function testnullsafe(bool $b, ?Derived $d):void {
  $c = new Derived();
  if ($b) $c = null;
  $x1 = applyAndReturn($c, $y ==> { return $y?->fld; });
  expect<Contra<?Derived>>($x1[1]);
  $x2 = applyAndReturn($c, $y ==> { return $y?->fld; });
  expect<Contra<?Base>>($x2[1]);
  $x3 = applyAndReturn($d, $y ==> { return $y?->fld; });
  expect<Contra<?Derived>>($x3[1]);
  $x4 = applyAndReturn($d, $y ==> { return $y?->fld; });
  expect<Contra<?Base>>($x4[1]);
}
/*
function testunion(bool $b):void {
  if ($b) {
    $u = new Base();
  } else {
    $u = new Unrelated();
  }
  $z1 = applyAndReturn($u, $y ==> $y->fld);
  expect<Contra<Derived>>($z1[1]);
  $z2 = applyAndReturn(new Derived(), $y ==> $y->fld);
  expect<Contra<Base>>($z2[1]);
  $z3 = applyAndReturn($d, $y ==> $y->fld);
  expect<Contra<Derived>>($z3[1]);
  $z4 = applyAndReturn($d, $y ==> $y->fld);
  expect<Contra<Base>>($z4[1]);
}
*/
