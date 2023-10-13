<?hh

class A {
  private int $x;
  private int $y;

  public function __construct(int $i) {
    switch ($i) {
      case 0:
        $this->x = 0;
        $this->y = 0;
        break;
      default:
        $this->x = 1;
        $this->y = 1;
        break;
    }
  }
}
