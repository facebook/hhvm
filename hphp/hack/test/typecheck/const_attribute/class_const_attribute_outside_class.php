<?hh // strict

<<__Const>>
class A {
  public int $denyMutateField;

  public function __construct() {
    $this->denyMutateField = 4;
  }
}

class B {
  public A $immutable;

  public function __construct() {
    $this->immutable = new A();
    $this->immutable->denyMutateField = 42;
  }
}
