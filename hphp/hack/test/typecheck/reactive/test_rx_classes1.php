<?hh // strict

class C {
  public function __construct(public int $f) {}
}

class A {
  <<__RxLocal>>
  public function f(C $c): int {
    return 1;
  }
}

class B extends A {
  // OK
  <<__RxLocal>>
  public function f(C $c): int {
    return 1;
  }
}

class C1 extends A {
  // not OK
  public function f(C $c): int {
    $c->f = 5;
    return 1;
  }
}
