<?hh // strict

class A {
  <<__MutableReturn>>
  public function f1(): A {
    return new A();
  }
}
