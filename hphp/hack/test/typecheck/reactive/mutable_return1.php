<?hh // strict

class A {
  <<__Rx>>
  public function __construct(private int $x) {
  }
  <<__Rx, __MutableReturn>>
  public static function get(): A {
    return new A(1);
  }
  <<__Rx, __Mutable>>
  public function setX(int $newX): void {
    $this->x = $newX;
  }
}

<<__Rx>>
function f(): void {
  $a = \HH\Rx\mutable(A::get());
  $a->setX(5);
}
