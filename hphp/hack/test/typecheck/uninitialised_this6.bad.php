<?hh

abstract class D {
  public int $i;
}

class C extends D {
  public function __construct() {
    $this->i;
    $this->i = 0;
  }
}
