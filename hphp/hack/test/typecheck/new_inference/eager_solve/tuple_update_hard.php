<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $item) { }
}
function test_shape_update((string,int) $p, int $i):
  Inv<(arraykey,int)> {
  $obj = new Inv($p);
  $r = $obj->item;
  $r[0] = $i;
  $obj->item = $r;
  return $obj;
}
