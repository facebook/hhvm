<?hh

class C {
  public int $i;
  public function __construct() {
    for ($i = 0; $this->isGood(); $this->i++) {
      $this;
    }
    $this->i = 1;
  }

  public function isGood(): bool {
    return true;
  }
}
