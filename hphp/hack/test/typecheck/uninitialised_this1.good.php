<?hh

abstract class D {
  public int $i;
}

class C extends D {
  public int $i = 0;
  public function __construct() {
    $this->i;
  }
}
