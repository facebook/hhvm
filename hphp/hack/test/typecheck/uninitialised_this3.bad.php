<?hh

class C {
  public int $i;
  public function __construct() {
    while (1 !== 2) {
      $this;
    }
    $this->i = 1;
  }
}
