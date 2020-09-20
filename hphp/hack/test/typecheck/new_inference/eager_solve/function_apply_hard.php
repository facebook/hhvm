<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $item) { }
}
function expectArraykey(arraykey $ak):void { }
function test_shape_update((function(string):int) $f, int $i):
  Inv<(function(string):arraykey)> {
  $obj = new Inv($f);
  $r = $obj->item;
  expectArraykey(($r)("a"));
  $obj->item = $r;
  return $obj;
}
