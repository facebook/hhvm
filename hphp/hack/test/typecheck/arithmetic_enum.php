<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

enum E : int as int {
  A = 3;
}


function testit<T as num>(T $x):void {
  $x = 3 + E::A;
  $y = E::A + E::A;
  $z = $x + $x;
  hh_show($x);
  hh_show($y);
  hh_show($z);
}
