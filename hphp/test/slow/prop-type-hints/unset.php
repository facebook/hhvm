<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public $x;
}

class A {
  public int $x;
  public C $y;
}

class B extends A {
  public function __construct() { $this->y = new C(); }
  public function test() {
    unset($this->y->x);
    unset($this->x);
  }
}

(new B())->test();
