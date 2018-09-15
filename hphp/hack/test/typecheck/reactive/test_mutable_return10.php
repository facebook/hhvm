<?hh // strict

class A {
  <<__Rx, __MutableReturn>>
  public function f(A $a): A {
    return new A();
  }
}

class B extends A {
  <<__Override, __Rx>>
  public function f(A $a): A {
    // not ok - mismatched __MutableReturn on base and overrided methods
    return $a;
  }
}
