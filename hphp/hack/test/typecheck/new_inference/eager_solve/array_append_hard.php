<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(public T $item) { }
}
function test_vec_append(vec<string> $v, int $i):Inv<vec<arraykey>> {
  // If we write Inv<vec<arraykey>> here then all is well
  $obj = new Inv($v);
  // So now we have
  //  vec<string> <: #1
  //  $obj: Inv<#1>
  $r = $obj->item;
  //  $r: #1
  $r[] = $i;
  // Now we try to solve for #1, assume (too strong) #1 = vec<string>
  // $r: vec<int|string>
  // $obj: Inv<vec<string>>
  // Type error!
  $obj->item = $r;
  return $obj;
}
