<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(shape('a' => int) $s, shape('b' => float, 'c' => string) $u): void {
  hh_show(Shapes::idx($s, 'a'));
  hh_show(Shapes::keyExists($s, 'a'));
  Shapes::removeKey(inout $u, 'b');
  hh_show($u);
  hh_show(Shapes::toArray($s));
  hh_show(Shapes::toDict($s));
  hh_show(Shapes::at($s, 'a'));
}

function dynamic_collection(dynamic $d): void {
  hh_show(Shapes::toArray($d));
  hh_show(Shapes::toDict($d));
}
