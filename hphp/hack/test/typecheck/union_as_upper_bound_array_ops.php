<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Base<T> {
  public function __construct(private T $item) { }
  public function get():T { return $this->item; }
}
class C<T as vec<int>> extends Base<T> {
}
class D<T as Vector<string>> extends Base<T> {
}

function test_flow<T>(mixed $m, bool $flag):arraykey {
  invariant($m is Base<_>, "Base");
  if ($flag) {
    invariant($m is D<_>, "D");
    $x = $m->get();
  } else {
    invariant($m is C<_>, "C");
    $x = $m->get();
  }
  // We've made ourself a T#1 <: vec<int> | Vector<string>
  // Now let's test some array operations
  $y = $x[2];
  $x[3] = "a";
  return $y;
}
