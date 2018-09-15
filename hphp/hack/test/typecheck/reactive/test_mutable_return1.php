<?hh // strict

class A {
  <<__Rx, __MutableReturn>>
  public function f1(): A {
    return new A();
  }
}
