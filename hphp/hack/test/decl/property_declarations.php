<?hh

class C {
  private Map<int, int>
    $map = Map {},
    $uninit;

  public function __construct() {
    $this->uninit = Map {};
  }
}
