<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(
  array<string,bool> $a,
  dict<int,bool> $d,
  darray<int,bool> $da):void {
  $x = $a[true];
  $y = $d[3.4];
  $z = $da[false];
}
