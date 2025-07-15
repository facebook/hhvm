<?hh

abstract class D {
}

class C extends D {
  public int $i;
  public function __construct() {
    $this->i;
    $this->i = 0;
  }
}
