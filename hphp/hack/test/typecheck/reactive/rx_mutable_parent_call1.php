<?hh // partial

class A {
  <<__Rx, __MutableReturn>>
  public function f() : A {
    return new A();
  }

  <<__Rx, __Mutable>>
  public function g(): void {
  }
}

class B extends A {
  <<__Rx>>
  public function h(): void {
    \HH\Rx\mutable(parent::f())->g();
  }
}
