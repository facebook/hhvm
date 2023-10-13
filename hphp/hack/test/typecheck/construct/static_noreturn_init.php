<?hh

class C {
  private int $x;

  private static function invariant_violation(): noreturn {
    throw new Exception();
  }

  public function __construct(bool $cond) {
    if ($cond) {
      self::invariant_violation();
    } else {
      $this->x = 4;
    }
  }
}
