<?hh //partial

class A {
  static public $x;
  public $y;

  public function __construct(string $y) {
    $this->y = $y;
  }

  public static function setX(int $x): void {
    A::$x = $x;
  }
}
