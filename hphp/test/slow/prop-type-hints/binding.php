<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public $x;
}

class A {
  public int $x;
  public C $y;

  public static int $s;
}

class B extends A {
  public function __construct() { $this->y = new C(); }
  public function test($p) {
    $this->y->x =& $p;
    $y =& $this->y->x;

    $this->x =& $p;
    $y =& $this->x;

    self::$s =& $p;
    $y =& self::$s;
  }
}

(new B())->test(123);
