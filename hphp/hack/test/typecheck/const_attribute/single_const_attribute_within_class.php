<?hh

class A {
  <<__Const>>
  public int $CA;

  public function __construct() {
    $this->CA = 4;
  }

  public function fail(): void {
    $this->CA = 42;
  }
}
