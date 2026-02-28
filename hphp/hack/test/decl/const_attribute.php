<?hh

<<__Const>>
class A {
  public int $x;

  <<__Const>>
  public int $y;

  public function __construct() {
    $this->x = 4;
    $this->y = 5;
  }
}

class B {
  <<__Const>>
  public int $x;

  public function __construct() {
    $this->x = 4;
  }
}

class C {
  public function __construct(<<__Const>> private int $x) {}
}
