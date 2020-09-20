<?hh // strict

class C {}

class A {
  <<__Rx, __MutableReturn>>
  public function f1(): A {
    // OK - returns fresh object
    return new A();
  }

  <<__Rx, __MutableReturn>>
  public function f2(): A {
    // OK
    return $this->f1();
  }
}
