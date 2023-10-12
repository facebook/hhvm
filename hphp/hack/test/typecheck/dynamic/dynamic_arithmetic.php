<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function testarith(dynamic $d, int $i, float $f, num $n):void {
  $x = $d + $i;
  hh_show($x);
  $x = $d - $i;
  hh_show($x);
  $x = $d * $i;
  hh_show($x);
  $d++;
  hh_show($d);
  $x = $d + $d;
  hh_show($x);
  $x = $d - $d;
  hh_show($x);
  $x = $d * $d;
  hh_show($x);
}
