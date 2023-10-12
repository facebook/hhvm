<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function testit(Vector<float> $v):Vector<float> {
  $v = new Vector($v);
  return $v->map($w ==> $w / 3);
}
