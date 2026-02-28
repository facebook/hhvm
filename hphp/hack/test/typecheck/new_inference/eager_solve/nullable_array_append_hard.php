<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $item) { }
}
function test_vec_append(?vec<string> $v, int $i):Inv<?vec<arraykey>> {
  $obj = new Inv($v);
  $r = $obj->item;
  if ($r !== null) {
    $r[] = $i;
    $obj->item = $r;
  }
  return $obj;
}
