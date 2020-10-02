<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(
  darray<string,bool> $a,
  dict<int,bool> $d,
  darray<int,bool> $da,
  dict<mixed,bool> $dm,
  KeyedContainer<int,string> $kc,
  ConstMap<int,bool> $cm,
  ImmMap<int, mixed> $im,
  mixed $k,
):void {
  $x = $a["a"];
  $y = $d["a"];
  $z = $da["a"];
  // Don't produce a second error here
  $zz = $dm[false];
  $xx = $kc["a"];
  $yy = $cm["a"];

  $_ = $kc[$k];
  $_ = $cm[$k];
  $_ = $im[$k];

  // These are legal because they're lvalues
  if ($a["a"]) {
    $a[2] = 3;
    $da["a"] = true;
    $da["b"] = true;
  } else {
    list($da['a'], $da[$kc['a']]) = tuple(1, 2);
  }
}
