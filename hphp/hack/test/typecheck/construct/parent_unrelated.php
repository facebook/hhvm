<?hh

abstract class C {
  private int $x;

  public function __construct(int $x) {
    $this->x = $x;
  }

  protected function init(): void {}
}

class D extends C {
  public function __construct() {
    parent::init(); // unrelated parent method
  }
}
