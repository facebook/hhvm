<?hh

class A {
  <<__Const>>
  public int $CA;

  public function __construct() {
    $this->CA = 4;
  }
}

class B extends A {
  public function __construct() {
    parent::__construct();
    $this->CA = 42; // override in subclass constructor allowed
  }

  public function fail(): void {
    $this->CA = 7; // not allowed
  }
}
