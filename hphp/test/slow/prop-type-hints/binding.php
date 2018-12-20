<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type FakeType1<T> = string;
type FakeType2<T> = mixed;
type FakeType3<T> = T;

class C {
  public $x;
}

class A {
  public int $x;
  public C $y;

  public FakeType1<string> $z1;
  public FakeType2<int> $z2;
  public FakeType3<bool> $z3;

  public static int $s;
}

class B extends A {
  public function __construct() { $this->y = new C(); }
  public function test($p) {
    $this->y->x =& $p;
    $y =& $this->y->x;

    $this->x =& $p;
    $y =& $this->x;

    $this->z1 =& $p;
    $y =& $this->z1;

    $this->z2 =& $p;
    $y =& $this->z2;

    $this->z3 =& $p;
    $y =& $this->z3;

    self::$s =& $p;
    $y =& self::$s;
  }
}

(new B())->test(123);
