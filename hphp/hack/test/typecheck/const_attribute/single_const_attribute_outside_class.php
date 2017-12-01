<?hh // strict

class A {
  <<__Const>>
  public int $CA;

  public function __construct() {
    $this->CA = 4;
  }
}

class B {
  public A $a;

  public function __construct() {
    $this->a = new A();
  }

  public function fail(): void {
    $this->a->CA = 42;
  }
}
