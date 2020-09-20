<?hh // strict

class A {
  private int $a;
  <<__Rx>>
  public function __construct(int $a) {
    $this->a = $a;
  }
}
