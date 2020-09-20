<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function coalesce<Tr, Ta as Tr, Tb as Tr>(?Ta $x , Tb $y): Tr {
  return $x ?? $y;
}
class C {}
function testit(?vec<C> $v1):void {
  $w = coalesce($v1, vec[]);
  hh_show($w);
}
