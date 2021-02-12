<?hh // strict
class C {}

class A {

  public function f1(): A {
    // OK - returns fresh object
    return new A();
  }


  public function f2(): A {
    // OK
    return $this->f1();
  }
}
