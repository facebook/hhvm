<?hh // strict
class A {

  public function f1(): A {
    return new A();
  }
}
