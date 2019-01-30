<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(
  array<string,bool> $a,
  dict<int,bool> $d,
  darray<int,bool> $da,
  KeyedContainer<int,bool> $kc,
  ConstMap<int,bool> $cm):void {
  $x = $a[true];
  $y = $d[3.4];
  $z = $da[false];
  $xx = $kc["a"];
  $yy = $cm["a"];
}
