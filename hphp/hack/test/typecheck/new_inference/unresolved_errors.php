<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function generic_function<T>():Map<int,T> {
  return Map{};
}

class Inv<Ti> {
  private vec<Ti> $arr = vec[];
  public function __construct() { }
  public function get():Ti { return $this->arr[0]; }
}

function foo():void {
  $m = Map {};
  $m[0]->foo();

  $v = Vector {};
  $v[0]->foo();

  $x = generic_function();
  $x[0]->foo();

  $a = varray[];
  $a[0]->foo();

  $x = new Inv();
  $x->get()->foo();
}
