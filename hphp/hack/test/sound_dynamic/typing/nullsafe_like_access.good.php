<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__SupportDynamicType>>
class A {
  public function __construct(public int $x) {}
}

<<__SupportDynamicType>>
  class MyVector<T as supportdyn<mixed>> {
  public function isEmpty() : bool { return false; }
  public function get() : ~T { throw new Exception("A"); }
  public function append(T $x) : void {}
  public function pess_append(~T $x) : void {}
}

function test(?A $a): void {
  $x = new MyVector();
  $y1 = $x->get();
  $y2 = $y1?->x;
  expect<?int>($y2);
  $x->pess_append($a);
}

function test2(?A $a): void {
  $x = new MyVector();
  expect($x->get()?->x);
  $x->pess_append($a);
}

function test3(?A $a): void {
  $x = new MyVector();
  expect<?int>($x->get()?->x);
  $x->append($a);
}

function test4(?A $a): void {
  $x = new MyVector();
  expect($x->get()?->x);
  $x->append($a);
}

<<__SupportDynamicType>>
  function expect<T as supportdyn<mixed>>(T $_): void {}
