<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function apply<T1,T2>((function(T1):T2) $f, T1 $x):T2 {
  return ($f)($x);
}
function ident<T1,T2>((function(T1):T2) $f):(function(T1):T2) {
  return $f;
}

function expectInt(int $x):void { }
function expectNum(num $x):void { }
function expectFloat(float $x):void { }
function expect<T>(T $x):void { }
function foo(num $n):void {
  $a = apply($x ==> $x+1, 3);
  expectInt($a);
  $a = apply($x ==> $x+1, 3.4);
  expectNum($a);
  $a = apply($x ==> $x+1, $n);
  expectNum($a);
  $a = apply($x ==> $x+1.2, 3);
  expectFloat($a);
  $a = apply($x ==> $x+$n, 3);
  expectNum($a);

  $f = ident($x ==> $x+1);
  expect<(function(int):int)>($f);
  $f = ident($x ==> $x+$x);
  expect<(function(int):int)>($f);
  $f = ident($x ==> $x+1.2);
  expect<(function(num):num)>($f);
}
