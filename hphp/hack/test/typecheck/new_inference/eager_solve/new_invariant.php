<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {
  public function __construct(private T $item) { }
  public function get(): T { return $this->item; }
}
class C {
  public function foo():void { }
}

function testit(C $y):void {
  $x = new Inv($y);
  $x->get()->foo();
}
