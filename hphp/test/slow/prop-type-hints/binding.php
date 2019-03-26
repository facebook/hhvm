<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type FakeType1<T> = string;
type FakeType2<T> = mixed;
type FakeType3<T> = T;

function by_ref(&$ref) {}

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
  public function test() {
    by_ref(&$this->y->x);
    by_ref(&$this->x);
    by_ref(&$this->z1);
    by_ref(&$this->z2);
    by_ref(&$this->z3);
    by_ref(&self::$s);
  }
}

(new B())->test();
