<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $item) { }
}
function test_shape_update(shape('a' => string) $s, int $i):
  Inv<shape('a' => arraykey)> {
  // If we write Inv<vec<arraykey>> here then all is well
  $obj = new Inv($s);
  $r = $obj->item;
  $r['a'] = $i;
  $obj->item = $r;
  return $obj;
}
